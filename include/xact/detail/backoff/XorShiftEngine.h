#pragma once

#include <cstdint>
#include <limits>
#include "xact/detail/macros.h"

namespace xact { namespace detail { namespace backoff {

class XorShiftEngine {
 public:

  // required for STL random engine concept
  using result_type = uint32_t;
  static result_type min() {
    return std::numeric_limits<result_type>::min();
  }
  static result_type max() {
    return std::numeric_limits<result_type>::max();
  }

 protected:
  uint32_t state_ {0};

  inline void doRound() {
    uint32_t x = state_;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state_ = x;
  }
 public:
  inline XorShiftEngine(uint32_t state) {
    seed(state);
  }
  inline void seed(uint32_t state) {
    XACT_DCHECK(state > 0);
    state_ = state;
    doRound();
  }
  inline uint32_t makeRand() {
    uint32_t result = state_;
    doRound();
    return result;
  }
  inline uint32_t operator()() {
    return makeRand();
  }
  inline void discard(size_t nSteps) {
    for (size_t i = 0; i < nSteps; i++) {
      makeRand();
    }
  }
};

}}} // xact::detail::backoff
