#include <gtest/gtest.h>
#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <vector>
#include <atomic>
#include <glog/logging.h>
#include "xact/detail/asm/tests.h"

using namespace std;

TEST(TestASMSanity, TestAdd2) {
  uint64_t x = 100;
  for (uint64_t y = 0; y < 100; y++) {
    EXPECT_EQ(x+y, xact_sanity_test_add2_64(x, y));
  }
}

TEST(TestASMSanity, TestEq) {
  uint64_t x = 100;
  uint64_t y = 100;
  for (size_t i = 0; i < 1000; i++) {
    EXPECT_TRUE(xact_sanity_test_eq_64(x, y));
    x++;
    y++;
  }
}
