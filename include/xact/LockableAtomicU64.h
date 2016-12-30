#pragma once
#include "xact/detail/asm/lockable_atomic_u64.h"
#include "xact/TransactionStatus.h"
#include <cstdint>

namespace xact {

class LockableAtomicU64Inspector;
class LockableAtomicU64Inspector2;

class LockableAtomicU64 {
 protected:
  xact_lockable_atomic_u64_t value_;
  friend class LockableAtomicU64Inspector;
  uint64_t* getPointer();
  uint64_t* valuePtr();
 public:
  LockableAtomicU64();
  LockableAtomicU64(uint64_t init);
  bool isLocked();
  bool tryLock();
  bool unlock();
  bool lock();
  TransactionStatus store(uint64_t init);
  TransactionStatus load(uint64_t *result);
  TransactionStatus compareExchange(uint64_t *expected, uint64_t desired);
  TransactionStatus fetchAdd(uint64_t *result, uint64_t addBy);
  TransactionStatus incr();
};

class LockableAtomicU64Inspector {
 protected:
   LockableAtomicU64& ref_;
 public:
  LockableAtomicU64Inspector(LockableAtomicU64& ref);
  uint64_t* getPointer();
};

} // xact
