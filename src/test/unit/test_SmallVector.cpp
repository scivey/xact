#include "xact/detail/SmallVector.h"
#include <vector>
#include <gtest/gtest.h>

using namespace xact;
using namespace xact::detail;

using namespace std;

TEST(TestSmallVector, TestSanity1) {
  SmallVector<uint64_t> nums;
  EXPECT_TRUE(nums.empty());
  for (size_t i = 0; i < 20; i++) {
    nums.push_back(i*10);
  }
  size_t currentSize = 20;
  while (currentSize > 0) {
    EXPECT_EQ(currentSize, nums.size());
    for (size_t i = 0; i < currentSize; i++) {
      auto val = nums.at(i);
      EXPECT_EQ(i*10, val);
    }
    nums.pop_back();
    currentSize--;
  }
}

