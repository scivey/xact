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
  std::vector<uint64_t> codes {10, 20, 30, 40};
  for (auto code: codes) {
    for (;;) {
      auto rc = getAbortReturn(code);
      if (rc != 0) {
        EXPECT_EQ(code+1, rc);
        break;
      }
    }
  }
}
