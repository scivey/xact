#include <gtest/gtest.h>
#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <vector>
#include <atomic>


#include "xact/atomic_ops/single.h"
#include "xact/AlignedBox.h"
#include "xact/AtomicU64.h"
#include "xact/fence.h"

using namespace std;
using xact::AtomicU64;


TEST(TestAtomicU64, TestFetchAddIfBetweenSingleThread1) {
  AtomicU64 atom {20};
  uint64_t result {0};

  EXPECT_TRUE(atom.fetchAddIfBetween(&result, 10, 0, 50));
  EXPECT_EQ(20, result);
  EXPECT_EQ(30, atom.load());

  EXPECT_FALSE(atom.fetchAddIfBetween(&result, 10, 80, 90));
  EXPECT_EQ(30, result);
  EXPECT_EQ(30, atom.load());
}

TEST(TestAtomicU64, TestFetchAddIfBetweenSingleThread2) {
  AtomicU64 atom {49};
  uint64_t result {0};
  EXPECT_TRUE(atom.fetchAddIfBetween(&result, 1, 48, 50));
  EXPECT_EQ(49, result);
  EXPECT_EQ(50, atom.load());

  EXPECT_FALSE(atom.fetchAddIfBetween(&result, 1, 48, 50));
  EXPECT_EQ(50, result);
  EXPECT_EQ(50, atom.load());
}


TEST(TestAtomicU64, TestFetchAddIfBetweenMultiThread) {
  static const size_t kNThreads = 8;
  for (size_t i = 0; i < 10; i++) {
    AtomicU64 atom {0};
    vector<unique_ptr<thread>> threads;
    for (size_t i = 0; i < kNThreads; i++) {
      threads.push_back(std::unique_ptr<thread>{new thread{[&atom]() {
        uint64_t result {0};
        for (;;) {
          if (!atom.fetchAddIfBetween(&result, 1, 0, 50000)) {
            if (result >= 50000) {
              break;
            }
          }
        }
      }}});
    }
    for (auto& t: threads) {
      t->join();
    }
    EXPECT_EQ(50000, atom.load());
  }
}
