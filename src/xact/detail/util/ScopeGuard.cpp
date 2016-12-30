#include <algorithm>
#include "xact/detail/util/ScopeGuard.h"

namespace xact { namespace detail { namespace util {

using void_func_t = ScopeGuard::void_func_t;

ScopeGuard::ScopeGuard(void_func_t&& func)
  : func_(std::forward<void_func_t>(func)) {
  token_.mark();
}

ScopeGuard::ScopeGuard(ScopeGuard&& other)
  : func_(std::move(other.func_)),
    token_(std::move(other.token_)),
    dismissed_(other.dismissed_) {}

ScopeGuard& ScopeGuard::operator=(ScopeGuard&& other) {
  std::swap(func_, other.func_);
  std::swap(token_, other.token_);
  std::swap(dismissed_, other.dismissed_);
  return *this;
}

void ScopeGuard::dismiss() {
  dismissed_ = true;
}

ScopeGuard::~ScopeGuard() {
  if (token_ && !dismissed_) {
    token_.clear();
    func_();
  }
}

}}} // xact::detail::util

