#include <random>
#include <gtest/gtest.h>
#include "xact/detail/backoff/ExponentialBackoff.h"

using namespace std;
using namespace xact::detail::backoff;


TEST(TestExponentialBackoff, TestWorkiness) {
  const uint32_t kLimit = 100000000;
  ExponentialBackoff backoff {10, kLimit, 67};

  uint32_t previous {0};
  for (size_t i = 0; i < 6; i++) {
    uint32_t current = backoff.next();
    EXPECT_LT(previous, current);
    previous = current;
  }
  for (size_t i = 0; i < 5000; i++) {
    EXPECT_LE(backoff.next(), kLimit);
  }
}

