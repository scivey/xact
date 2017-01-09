#pragma once
#include "xact/detail/asm/rw_seqlock.h"
#include "xact/detail/asm/rw_seqlock_ops.h"

#include "xact/detail/macros.h"

namespace xact { namespace detail {

class RWSeqLockValue {
 protected:
  uint64_t value_ {0};
 public:
  inline RWSeqLockValue(){}
  inline RWSeqLockValue(uint64_t val): value_(val) {}

  inline uint64_t getValue() const {
    return value_;
  }
  inline uint64_t getVersion() const {
    return xact_rw_seqlock_get_version_from_uint64_t(value_);
  }
  inline bool isWriteLocked() const {
    return xact_rw_seqlock_is_write_locked_from_uint64_t(value_);
  }
  inline bool isReadLocked() const {
    return xact_rw_seqlock_is_read_locked_from_uint64_t(value_);
  }
  inline bool isLocked() const {
    return isWriteLocked() || isReadLocked();
  }
};


template<typename TSub>
class RWSeqLockBase {
 protected:
  using sub_type = TSub;
  TSub* getSub() {
    return static_cast<TSub*>(this);
  }
 public:
  inline void writeLock() {
    XACT_DCHECK(xact_rw_seqlock_write_lock(getSub()->getLockPointer()) == 0);
    XACT_RW_BARRIER();    
  }
  inline void writeUnlock() {
    XACT_DCHECK(xact_rw_seqlock_write_unlock(getSub()->getLockPointer()) == 0);
    XACT_RW_BARRIER();
  }
  inline bool tryWriteLock() {
    auto result = xact_rw_seqlock_try_write_lock(getSub()->getLockPointer());
    XACT_RW_BARRIER();
    return result;
  }
  inline bool isWriteLocked() {
    return getValue().isWriteLocked();
  }

  inline void readLock() {
    XACT_DCHECK(xact_rw_seqlock_read_lock(getSub()->getLockPointer()) == 0);
    XACT_RW_BARRIER();    
  }
  inline void readUnlock() {
    XACT_DCHECK(xact_rw_seqlock_read_unlock(getSub()->getLockPointer()) == 0);
    XACT_RW_BARRIER();    
  }    
  inline bool tryReadLock() {
    auto result = xact_rw_seqlock_try_read_lock(getSub()->getLockPointer());
    XACT_RW_BARRIER();
    return result;
  }
  inline bool isReadLocked() {
    return getValue().isReadLocked();
  }
  inline bool isLocked() {
    return getValue().isLocked();
  }
  inline RWSeqLockValue getValue() {
    auto result = RWSeqLockValue(getRawValue());
    XACT_RW_BARRIER();
    return result;
  }
  inline uint64_t getRawValue() {
    auto result = xact_rw_seqlock_get_raw_value(getSub()->getLockPointer());
    XACT_RW_BARRIER();
    return result;
  }
  inline uint64_t getVersion() {
    return getValue().getVersion();
  }
};


class RWSeqLock: public RWSeqLockBase<RWSeqLock> {
 protected:
  xact_rw_seqlock_t lock_;
  RWSeqLock(const RWSeqLock&) = delete;
  RWSeqLock& operator=(const RWSeqLock&) = delete;
 public:
  RWSeqLock() {
    XACT_DCHECK(xact_rw_seqlock_init(&lock_) == 0);
    XACT_MFENCE_BARRIER();    
  }
  xact_rw_seqlock_t* getLockPointer() {
    return &lock_;
  }
} __attribute__((aligned(16)));


class RWSeqLockRef: public RWSeqLockBase<RWSeqLockRef> {
 protected:
  xact_rw_seqlock_t *lock_ {nullptr};

 public:
  RWSeqLockRef(xact_rw_seqlock_t *lockPtr): lock_(lockPtr) {}
  xact_rw_seqlock_t* getLockPointer() {
    return lock_;
  }
};



}} // xact::detail
