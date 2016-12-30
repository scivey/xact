#include "xact/detail/util/UniqueToken.h"
#include <algorithm>

namespace xact { namespace detail { namespace util {

UniqueToken::UniqueToken(bool value): value_(value){}

UniqueToken::UniqueToken(): value_(false) {}

void UniqueToken::mark() noexcept {
  value_ = true;
}

void UniqueToken::clear() noexcept {
  value_ = false;
}

UniqueToken::UniqueToken(UniqueToken&& other): value_(other.value_) {
  other.value_ = false;
}

UniqueToken& UniqueToken::operator=(UniqueToken&& other) {
  std::swap(value_, other.value_);
  return *this;
}

bool UniqueToken::good() const noexcept {
  return value_;
}

UniqueToken::operator bool() const noexcept {
  return good();
}

}}} // xact::detail::util


