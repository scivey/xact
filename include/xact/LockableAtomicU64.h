#pragma once
#include <cstdint>
#include "xact/TransactionStatus.h"
#include "xact/single/SingleTransaction.h"
#include "xact/detail/RWSeqLock.h"
#include "xact/detail/asm/lockable_atomic_u64.h"
#include "xact/detail/asm/lockable_atomic_u64_ops.h"
#include "xact/detail/asm/util.h"
#include "xact/detail/macros.h"

namespace xact {


namespace detail {
class LockableAtomicU64Inspector;
}

class LockableAtomicU64 {
 public:
  using SingleTransaction = single::SingleTransaction;
 protected:
  xact_lockable_atomic_u64_t value_;
  friend class detail::LockableAtomicU64Inspector;

  inline uint64_t* getPointer() {
    return valuePtr();
  }
  inline uint64_t* valuePtr() {
    return (uint64_t*) &value_;
  }
  inline void init(uint64_t initVal = 0) {
    xact_lockable_atomic_u64_init(&value_, initVal);
    XACT_MFENCE_BARRIER();
  }
  inline detail::RWSeqLockRef getLockRef() {
    return detail::RWSeqLockRef {&value_.seqlock};
  }
 public:
  inline LockableAtomicU64() {
    init();
  }
  inline LockableAtomicU64(uint64_t initVal) {
    init(initVal);
  }
  inline bool isLocked() {
    return getLockRef().isLocked();
  }

  inline SingleTransaction makeStore(uint64_t init) {
    return SingleTransaction::store(&value_, init);
  }
  inline TransactionStatus store(uint64_t init) {
    return makeStore(init).execute();
  }
  inline SingleTransaction makeLoad(uint64_t *result) {
    return SingleTransaction::load(&value_, result);
  }
  inline TransactionStatus load(uint64_t *result) {
    return makeLoad(result).execute();
  }
  inline SingleTransaction makeCompareExchange(uint64_t *expected,
      uint64_t desired) {
    return SingleTransaction::compareExchange(
      &value_, expected, desired
    );
  }
  inline TransactionStatus compareExchange(uint64_t *expected, uint64_t desired) {
    return makeCompareExchange(expected, desired).execute();
  }
  inline SingleTransaction makeFetchAdd(uint64_t *result, uint64_t addBy) {
    return SingleTransaction::fetchAdd(
      &value_, result, addBy
    );
  }
  inline TransactionStatus fetchAdd(uint64_t *result, uint64_t addBy) {
    return makeFetchAdd(result, addBy).execute();
  }
};

namespace detail {
class LockableAtomicU64Inspector {
 protected:
   LockableAtomicU64& ref_;
 public:
  inline LockableAtomicU64Inspector(LockableAtomicU64& ref): ref_(ref){}
  inline uint64_t* getPointer() {
    return ref_.getPointer();
  }
  inline detail::RWSeqLockRef getLockRef() {
    return ref_.getLockRef();
  }
};
} // detail


} // xact
