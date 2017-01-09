#pragma once
#include "xact/detail/asm/lockable_atomic_u64.h"
#include "xact/detail/asm/lockable_atomic_u64_ops.h"
#include "xact/detail/NamedType.h"
#include "xact/detail/util/ScopeGuard.h"
#include "xact/detail/RWSeqLock.h"
#include "xact/TransactionStatus.h"

namespace xact { namespace single {

class SingleTransaction {
 public:
  enum class Type {
    NOOP,
    LOAD,
    STORE,
    COMPARE_EXCHANGE,
    FETCH_ADD
  };
  using atom_t = xact_lockable_atomic_u64_t;

 protected:
  struct Params {
    Type ttype {Type::NOOP};
    atom_t *target {nullptr};
    uint64_t arg1 {0};
    uint64_t arg2 {0};
  };
  Params params_;

  inline static SingleTransaction makeInstance(Type ttype, atom_t *target) {
    SingleTransaction instance;
    instance.params_.ttype = ttype;
    instance.params_.target = target;
    return instance;
  }

  inline static SingleTransaction makeInstance(Type ttype,
      atom_t *target, uint64_t arg1) {
    SingleTransaction instance;
    instance.params_.ttype = ttype;
    instance.params_.target = target;
    instance.params_.arg1 = arg1;
    return instance;
  }

  inline static SingleTransaction makeInstance(Type ttype,
      atom_t *target, uint64_t *arg1) {
    return makeInstance(ttype, target, (uint64_t) arg1);
  }

  inline static SingleTransaction makeInstance(Type ttype,
      atom_t *target, uint64_t *arg1, uint64_t arg2) {
    SingleTransaction instance;
    instance.params_.ttype = ttype;
    instance.params_.target = target;
    instance.params_.arg1 = (uint64_t) arg1;
    instance.params_.arg2 = arg2;
    return instance;
  }  

 public:
  inline SingleTransaction(){}

  inline static SingleTransaction load(atom_t *target, uint64_t *result) {
    return makeInstance(Type::LOAD, target, result);
  }

  inline static SingleTransaction store(atom_t *target, uint64_t value) {
    return makeInstance(Type::STORE, target, value);
  }

  inline static SingleTransaction compareExchange(atom_t *target, uint64_t *expected,
      uint64_t desired) {
    return makeInstance(Type::COMPARE_EXCHANGE, target, expected, desired);
  }

  inline static SingleTransaction fetchAdd(atom_t *target, uint64_t *result,
      uint64_t addBy) {
    return makeInstance(Type::FETCH_ADD, target, result, addBy);
  }

 protected:

  using UseTSX = detail::NamedType<bool, struct UseTSXTag>;
  using RWSeqLockRef = detail::RWSeqLockRef;

  inline TransactionStatus oldSlowExecuteStore(UseTSX useTsx) {
    if (useTsx.value()) {
      return transactionStatusFromRax(
        xact_lockable_atomic_u64_store_tsx(params_.target, params_.arg1)
      );
    }
    throw std::runtime_error("bad!");
  }
  inline TransactionStatus executeStore(UseTSX useTsx) {
    return transactionStatusFromAbortCode(
      xact_lockable_atomic_u64_store_cmpxchg16b(params_.target, params_.arg1)
    );
    // if (useTsx.value()) {
    //   return transactionStatusFromRax(
    //     xact_lockable_atomic_u64_store_tsx(params_.target, params_.arg1)
    //   );
    // }
    // throw std::runtime_error("bad!");
  }

  inline TransactionStatus oldSlowExecuteLoad1(UseTSX useTsx) {
    return transactionStatusFromRax(xact_lockable_atomic_u64_load_tsx(
      params_.target, (uint64_t*) params_.arg1
    ));
  }
  inline TransactionStatus oldSlowExecuteLoad2(UseTSX useTsx) {
    ((void) useTsx);
    auto target = params_.target;
    RWSeqLockRef slockRef {&target->seqlock};
    for (;;) {
      auto slockVal1 = slockRef.getValue();
      XACT_RW_BARRIER();
      if (slockVal1.isWriteLocked()) {
        return TransactionStatus::RESOURCE_LOCKED;
      }
      uint64_t atomValue = xact_lockable_atomic_u64_load_raw_value_dangerously(target);
      XACT_RW_BARRIER();
      auto slockVal2 = slockRef.getValue();
      if (slockVal2.isWriteLocked()) {
        return TransactionStatus::RESOURCE_LOCKED;
      }
      if (slockVal1.getVersion() != slockVal2.getVersion()) {
        // we made an inconsistent read
        continue;
      }
      auto resultPtr = (uint64_t*) params_.arg1;
      *resultPtr = atomValue;
      return TransactionStatus::OK;      
    }
  }
  inline TransactionStatus executeLoad(UseTSX useTsx) {
    ((void) useTsx);
    auto target = params_.target;
    auto resultPtr = (uint64_t*) params_.arg1;
    auto rc = xact_lockable_atomic_u64_load_mvcc(target, resultPtr);
    return transactionStatusFromAbortCode(rc);
  }

  inline TransactionStatus executeOldSlowCompareExchange(UseTSX useTsx) {
    if (useTsx.value()) {
      return transactionStatusFromRax(xact_lockable_atomic_u64_compare_exchange(
        params_.target, (uint64_t*) params_.arg1, params_.arg2
      ));
    }
    throw std::runtime_error("bad!");
  }

  inline TransactionStatus executeCompareExchange(UseTSX useTsx) {
    auto rc = xact_lockable_atomic_u64_compare_exchange_cmpxchg16b(
      params_.target, (uint64_t*) params_.arg1, params_.arg2
    );
    return transactionStatusFromAbortCode(rc);
  }

  inline TransactionStatus oldSlowExecuteFetchAdd(UseTSX useTsx) {
    if (useTsx.value()) {
      return transactionStatusFromRax(
        xact_lockable_atomic_u64_fetch_add_tsx(
          params_.target,
          (uint64_t*) params_.arg1,
          params_.arg2
        )
      );
    }
    throw std::runtime_error("bad!");
  }

  inline TransactionStatus executeFetchAdd(UseTSX useTsx) {
    return transactionStatusFromAbortCode(
      xact_lockable_atomic_u64_fetch_add_cmpxchg16b(
        params_.target,
        (uint64_t*) params_.arg1,
        params_.arg2
      )
    );
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
      case Type::FETCH_ADD:
        return executeFetchAdd(useTsx);
    };
  }
 public:
  inline TransactionStatus execute() {
    return pExecute(UseTSX{true});
  }
  inline TransactionStatus executeWithLocksHeld() {
    return pExecute(UseTSX{false});
  }
  inline TransactionStatus lockAndExecute() {
    using atom_t = xact_lockable_atomic_u64_t;
    return TransactionStatus::EMPTY;
    // RWSeqLockRef slockRef {&params_.target->seqlock};
    // XACT_RW_BARRIER();
    // slockRef.readLock();
    // // auto guard = detail::util::makeGuard([&slockRef]() {
    // //   slockRef.readUnlock();
    // // });
    // XACT_RW_BARRIER();
    // auto result = this->executeWithLocksHeld();
    // slockRef.unlock();
  }
};

}}
