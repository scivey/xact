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


uint64_t getAbortReturn(uint64_t code) {
  return xact_tsx_test_returned_abort_value(code); 
}

TEST(TestTSXAssumptions, TestAbortReturnCode) {
  EXPECT_EQ(11, getAbortReturn(10));
  EXPECT_EQ(21, getAbortReturn(20));
  EXPECT_EQ(31, getAbortReturn(30));
  EXPECT_EQ(41, getAbortReturn(40));
}
