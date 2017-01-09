#pragma once
#include <tuple>
#include "xact/detail/asm/lockable_atomic_u64.h"
#include "xact/detail/asm/lockable_atomic_u64_ops.h"
#include "xact/detail/NamedType.h"
#include "xact/TransactionStatus.h"

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
  struct Argument {
    atom_t *target {nullptr};
    uint64_t arg1;
    uint64_t arg2;
  };
  static_assert(sizeof(Argument) == 24, "Argument is 24 bytes wide");

  struct Params {
    Type ttype {Type::NOOP};
    std::vector<Argument> arguments;
  };
  Params params_;

 public:
  inline MultiTransaction(){}

  using load_param_t = std::pair<atom_t*, uint64_t*>;

  template<typename TContainer,
    typename = typename std::enable_if<
      std::is_same<
        decltype(std::declval<TContainer>().front()),
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
        decltype(std::declval<TContainer>().front()),
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


};

}} // xact::multi
