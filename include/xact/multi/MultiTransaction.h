#pragma once
#include <tuple>
#include <glog/logging.h>
#include "xact/TransactionStatus.h"
#include "xact/detail/asm/lockable_atomic_u64.h"
#include "xact/detail/asm/lockable_atomic_u64_ops.h"
#include "xact/detail/asm/multi.h"
#include "xact/detail/asm/multi_ops.h"
#include "xact/detail/SmallVector.h"
#include "xact/detail/named_types.h"
#include "xact/detail/LockManager.h"
#include "xact/detail/util/ScopeGuard.h"

namespace xact { namespace multi {

class MultiTransaction {
 public:
  enum class Type {
    NOOP,
    LOAD,
    STORE,
    COMPARE_EXCHANGE
  };
  using atom_t = xact_lockable_atomic_u64_t;

 protected:
  using Argument = xact_multi_atomic_op_argument_t;
  static_assert(sizeof(Argument) == 24, "Argument is 24 bytes wide");

  struct Params {
    Type ttype {Type::NOOP};
    detail::SmallVector<Argument> arguments;
  };
  Params params_;

 public:
  inline MultiTransaction(){}

  using load_param_t = std::pair<atom_t*, uint64_t*>;

  template<typename TContainer,
    typename = typename std::enable_if<
      std::is_same<
        typename TContainer::value_type,
        load_param_t
      >::value,
      TContainer>::type>
  inline static MultiTransaction load(const TContainer& refs) {
    MultiTransaction instance;
    instance.params_.ttype = Type::LOAD;
    for (const auto& loadParam: refs) {
      Argument arg;
      arg.target = loadParam.first;
      arg.arg1 = (uint64_t) loadParam.second;
      instance.params_.arguments.emplace_back(arg);
    }
    return instance;
  }


  using store_param_t = std::pair<atom_t*, uint64_t>;
  template<typename TContainer,
    typename = typename std::enable_if<
      std::is_same<
        typename TContainer::value_type,
        store_param_t
      >::value,
      TContainer>::type>
  inline static MultiTransaction store(const TContainer& refs) {
    MultiTransaction instance;
    instance.params_.ttype = Type::STORE;
    for (const auto& storeParam: refs) {
      Argument arg;
      arg.target = storeParam.first;
      arg.arg1 = storeParam.second;
      instance.params_.arguments.emplace_back(arg);
    }
    return instance;
  }


  using cas_param_t = std::tuple<atom_t*, uint64_t*, uint64_t>;
  template<typename TContainer,
    typename = typename std::enable_if<
      std::is_same<
        decltype(std::declval<TContainer>().front()),
        cas_param_t
      >::value,
      TContainer>::type>
  inline static MultiTransaction compareExchange(const TContainer& refs) {
    MultiTransaction instance;
    instance.params_.ttype = Type::COMPARE_EXCHANGE;
    for (const auto& casParam: refs) {
      Argument arg;
      arg.target = std::get<0>(casParam);
      arg.arg1 = (uint64_t) std::get<1>(casParam);
      arg.arg2 = std::get<2>(casParam);
      instance.params_.arguments.emplace_back(arg);
    }
    return instance;
  }

 protected:
  using UseTSX = detail::UseTSX;

  inline TransactionStatus executeStore(UseTSX useTsx) {
    auto& args = params_.arguments;
    CHECK(args.size() == 2);
    if (useTsx.value()) {
      auto rc = xact_multi_u64_store_2_tsx(&args[0], &args[1]);
      return transactionStatusFromRax(rc);
    } else {
      auto rc = xact_multi_u64_store_2_mvcc_with_locks_held(&args[0], &args[1]);
      return transactionStatusFromAbortCode(rc);
    }
  }
  inline TransactionStatus executeLoad(UseTSX useTsx) {
    auto& args = params_.arguments;
    CHECK(args.size() == 2);
    auto rc = xact_multi_u64_load_2_mvcc(&args[0], &args[1]);
    return transactionStatusFromAbortCode(rc);
  }
  inline TransactionStatus executeCompareExchange(UseTSX useTsx) {
    throw std::runtime_error("bad!");
  }

  inline TransactionStatus pExecute(UseTSX useTsx) {
    switch(params_.ttype) {
      case Type::NOOP:
        return TransactionStatus::EMPTY;
      case Type::STORE:
        return executeStore(useTsx);
      case Type::LOAD:
        return executeLoad(useTsx);
      case Type::COMPARE_EXCHANGE:
        return executeCompareExchange(useTsx);
    };
  }
  inline TransactionStatus executeWithLocksHeld() {
    return pExecute(UseTSX{false});
  }

 public:
  inline TransactionStatus execute() {
    return pExecute(UseTSX{true});
  }
  inline TransactionStatus lockAndExecute() {
    using atom_vec = detail::SmallVector<atom_t*>;
    atom_vec atoms;
    atoms.reserve(params_.arguments.size());
    for (auto& arg: params_.arguments) {
      atoms.push_back(arg.target);
    }
    atom_t** dataPtr = atoms.data();
    auto lockManager = detail::LockManager::create(dataPtr, atoms.size());
    if (params_.ttype == Type::NOOP) {
      return TransactionStatus::EMPTY;
    } else if(params_.ttype == Type::LOAD) {
      // read-only operation
      for (;;) {
        if (lockManager.tryLockForRead()) {
          auto guard = detail::util::makeGuard([&lockManager]() {
            lockManager.unlockFromRead();
          });
          return this->executeWithLocksHeld();
        }
      }
    } else {
      for (;;) {
        if (lockManager.tryLockForWrite()) {
          auto guard = detail::util::makeGuard([&lockManager]() {
            lockManager.unlockFromWrite();
          });
          return this->executeWithLocksHeld();
        }
      }      
    }
  }

};

}} // xact::multi
