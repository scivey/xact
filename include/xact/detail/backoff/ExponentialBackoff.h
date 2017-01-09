#pragma once

#include <cstdint>
#include <random>
#include "xact/detail/backoff/XorShiftEngine.h"

namespace xact { namespace detail { namespace backoff {


class ExponentialBackoff {
 protected:
  uint32_t start_ {0};
  uint32_t max_ {0};
  uint32_t current_ {0};
  XorShiftEngine rand_;
  uint32_t makeJitter(uint32_t num) {
    if (num < 4) {
      num = 4;
    }
    uint32_t maxJitter = std::min<uint32_t>(num/2, 100);
    uint32_t jitter = std::uniform_int_distribution<uint32_t>(1, maxJitter)(rand_);
    return jitter;
  }
  uint32_t makeJitteredMax() {
    // we don't ever want to exceed the max limit
    // specified in the constructor, but we still
    // want the benefits of jitter.
    // so when current is >= max, we subtract jitter instead.
    uint32_t result = max_;
    auto jitter = makeJitter(result);

    // underflow shouldn't happen unless the limit was set stupidly low
    if (XACT_LIKELY(jitter < result)) {
      result -= jitter;
    }
    return result;
  }
 public:
  inline ExponentialBackoff(uint32_t start, uint32_t lim, uint32_t seed)
    : start_(start), max_(lim), current_(start), rand_(seed) {
    reset();
  }
  inline uint32_t next() {
    uint32_t current = current_;
    if (current >= max_) {
      return makeJitteredMax();
    }
    uint32_t next = current | (current << 1);
    next += makeJitter(next);
    if (XACT_UNLIKELY(next > max_)) {
      next = max_;
    }
    current_ = next;
    return current;
  }
  inline void reset() {
    current_ = start_ + makeJitter(start_);
  }
};

}}} // xact::detail::backoff
