#include "xact/LockableAtomicU64.h"
#include "xact/TransactionStatus.h"
#include "xact/detail/asm/lockable_atomic_u64.h"
#include "xact/detail/asm/lockable_atomic_u64_ops.h"
#include "xact/detail/asm/util.h"
#include "xact/detail/macros.h"

#include <glog/logging.h>
#include <bitset>
#include <iostream>

namespace xact {

LockableAtomicU64::LockableAtomicU64() {
  xact_lockable_atomic_u64_init(&value_, 0);
  XACT_MFENCE_BARRIER();
}

uint64_t* LockableAtomicU64::valuePtr() {
  return (uint64_t*) &value_;
}

bool LockableAtomicU64::isLocked() {
  return xact_lockable_atomic_u64_is_locked(&value_) == 1;
}

bool LockableAtomicU64::tryLock() {
  return xact_lockable_atomic_u64_trylock(&value_) == 1;
}

bool LockableAtomicU64::unlock() {
  return xact_lockable_atomic_u64_unlock(&value_) == 0;
}

bool LockableAtomicU64::lock() {
  return xact_lockable_atomic_u64_lock(&value_) == 0;
}


LockableAtomicU64::LockableAtomicU64(uint64_t init) {
  xact_lockable_atomic_u64_init(&value_, init);
}

TransactionStatus LockableAtomicU64::store(uint64_t init) {
  auto rc = xact_lockable_atomic_u64_store(&value_, init);
  return transactionStatusFromRax(rc);
}


TransactionStatus LockableAtomicU64::load(uint64_t *result) {
  auto rc = xact_lockable_atomic_u64_load(&value_, result);
  return transactionStatusFromRax(rc);
}


TransactionStatus LockableAtomicU64::compareExchange(uint64_t *expected,
    uint64_t desired) {
  auto rc = xact_lockable_atomic_u64_compare_exchange(&value_, expected, desired);
  return transactionStatusFromRax(rc);
}

TransactionStatus LockableAtomicU64::fetchAdd(uint64_t *result, uint64_t addBy) {
  return transactionStatusFromRax(
    xact_lockable_atomic_u64_fetch_add(&value_, result, addBy)
  );
}

TransactionStatus LockableAtomicU64::incr() {
  return transactionStatusFromRax(
    xact_lockable_atomic_u64_incr(&value_)
  );
}

uint64_t* LockableAtomicU64::getPointer() {
  return valuePtr();
}


LockableAtomicU64Inspector::LockableAtomicU64Inspector(LockableAtomicU64& ref): ref_(ref){}

uint64_t* LockableAtomicU64Inspector::getPointer() {
  return ref_.getPointer();
}



} // xact

