#include "xact/detail/util/PThreadSpinLock.h"
#include "xact/detail/util/ScopeGuard.h"
#include "xact/detail/macros.h"
#include "xact/detail/fence.h"

namespace xact { namespace detail {namespace util {

PThreadSpinLock::PThreadSpinLock(PThreadSpinLock&& other)
  : spinLock_(other.spinLock_) {
  other.spinLock_ = nullptr;
  mFence();
}

PThreadSpinLock::PThreadSpinLock(pthread_spinlock_t* spinLock)
  : spinLock_(spinLock) {
  mFence();
  XACT_CHECK(!!spinLock_);
}

PThreadSpinLock& PThreadSpinLock::operator=(PThreadSpinLock&& other) {
  std::swap(spinLock_, other.spinLock_);
  mFence();
  return *this;
}

PThreadSpinLock PThreadSpinLock::create() {
  pthread_spinlock_t* spinner {nullptr};
  auto guard = util::makeGuard([&spinner](){
    if (spinner) {
      delete spinner;
    }
  });
  spinner = new pthread_spinlock_t;
  XACT_CHECK(pthread_spin_init(spinner, PTHREAD_PROCESS_PRIVATE) == 0);
  guard.dismiss();
  return PThreadSpinLock {spinner};
}

bool PThreadSpinLock::good() const {
  return !!spinLock_;
}

PThreadSpinLock::operator bool() const {
  return good();
}

bool PThreadSpinLock::try_lock() {
  XACT_DCHECK(good());
  return pthread_spin_trylock(spinLock_) == 0;
}

void PThreadSpinLock::lock() {
  XACT_DCHECK(good());
  XACT_DCHECK(pthread_spin_lock(spinLock_) == 0);
}

void PThreadSpinLock::unlock() {
  XACT_DCHECK(good());
  XACT_DCHECK(pthread_spin_unlock(spinLock_) == 0);
}

}}} // xact::detail::util
