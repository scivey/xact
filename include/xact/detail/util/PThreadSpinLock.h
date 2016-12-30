#pragma once

#include <pthread.h>

namespace xact { namespace detail { namespace util {

class PThreadSpinLock {
 protected:
  pthread_spinlock_t *spinLock_ {nullptr};
  PThreadSpinLock(const PThreadSpinLock&) = delete;
  PThreadSpinLock& operator=(const PThreadSpinLock&) = delete;
  PThreadSpinLock();
  PThreadSpinLock(pthread_spinlock_t*);
 public:
  PThreadSpinLock(PThreadSpinLock&&);
  PThreadSpinLock& operator=(PThreadSpinLock&&);
  static PThreadSpinLock create();
  bool good() const;
  explicit operator bool() const;
  bool try_lock();
  void lock();
  void unlock();
};

}}} // xact::detail::util
