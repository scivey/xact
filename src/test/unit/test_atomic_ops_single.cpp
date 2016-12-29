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

using namespace xact::atomic_ops;
using namespace std;
using xact::mFence;
using Box64 = xact::AtomicU64::box_t;


void sanityCheckBox(uint64_t *ptr, uint64_t initial, const string& msg) {
  EXPECT_EQ(initial, loadU64S(ptr));
  storeU64S(ptr, 10);
  EXPECT_EQ(10, loadU64S(ptr));
  EXPECT_TRUE(casU64S(ptr, 10, 20));
  EXPECT_EQ(20, loadU64S(ptr));
  EXPECT_FALSE(casU64S(ptr, 999, 5000));
  EXPECT_EQ(20, loadU64S(ptr));
  EXPECT_EQ(20, fetchAddU64S(ptr, 5));
  EXPECT_EQ(25, loadU64S(ptr));
  EXPECT_EQ(25, fetchSubU64S(ptr, 3));
  EXPECT_EQ(22, loadU64S(ptr));
}

TEST(TestU64SingleOps, TestNoContentionOnStackOne) {
  Box64 box {0};
  sanityCheckBox(box.ptr(), 0, "single box64 on stack");
}

TEST(TestU64SingleOps, TestNoContentionOnStackMore) {
  const size_t kNumBoxes = 20;
  Box64 boxes[kNumBoxes];
  for (size_t i = 0; i < kNumBoxes; i++) {
    auto& box = boxes[i];
    uint64_t initial = 0;
    storeU64S(box.ptr(), initial);
    sanityCheckBox(box.ptr(), initial, "multi box64 on stack");
  }
}

TEST(TestU64SingleOps, TestNoContentionOnHeap) {
  vector<unique_ptr<Box64>> boxes;
  const size_t kNumBoxes = 20;
  boxes.reserve(kNumBoxes);
  for (size_t i = 0; i < kNumBoxes; i++) {
    boxes.push_back(unique_ptr<Box64>{
      new Box64{0}
    });
  }
  for (auto& uptr: boxes) {
    sanityCheckBox(uptr->ptr(), 0, "box64 on heap");
  }
}


TEST(TestU64SingleOps, TestCASSequence) {
  Box64 box {0};
  auto boxPtr = box.ptr();
  storeU64S(boxPtr, 0);
  const size_t kIncrs = 99573;
  thread t1([&boxPtr]() {
    for (size_t i = 0; i < kIncrs; i++) {
      for (;;) {
        auto current = loadU64S(boxPtr);
        auto desired = current + 1;
        if (casU64S(boxPtr, current, desired)) {
          break;
        }
      }
    }
  });
  t1.join();
  EXPECT_EQ(kIncrs, loadU64S(boxPtr));
}


static const size_t kNumThreads = 4;
static const size_t kIncrsPerThread = 50000;

TEST(TestU64SingleOps, TestThreadsOnStack) {
  Box64 box {0};
  std::atomic<size_t> counter {0};
  auto boxPtr = box.ptr();
  storeU64S(boxPtr, 0);
  counter.load();
  vector<unique_ptr<thread>> threads;
  for (size_t i = 0; i < kNumThreads; i++) {
    threads.push_back(std::unique_ptr<thread>{ new thread{[boxPtr, &counter]() {
      size_t nSuccess = 0;

      // this is an intentionally stupid fetch-add
      // to expose problems that an atomic incr may not
      for (size_t i = 0; i < kIncrsPerThread; i++) {
        for (;;) {
          auto expected = loadU64S(boxPtr);
          counter.load();
          auto desired = expected + 1;
          if (casU64S(boxPtr, expected, desired)) {
            counter.fetch_add(1);
            break;
          }
        }
      }
    }}});
  }
  for (auto &t: threads) {
    t->join();
  }
  uint64_t expected = kNumThreads * kIncrsPerThread;
  EXPECT_EQ(expected, counter.load());
  EXPECT_EQ(expected, loadU64S(box.ptr()));
}

TEST(TestU64SingleOps, TestThreadsOnFetchAdd) {
  Box64 box {0};
  auto boxPtr = box.ptr();
  storeU64S(boxPtr, 0);
  vector<unique_ptr<thread>> threads;
  for (size_t i = 0; i < kNumThreads; i++) {
    threads.push_back(std::unique_ptr<thread>{ new thread{[boxPtr]() {
      size_t nSuccess = 0;

      // this is an intentionally stupid fetch-add
      // to expose problems that an atomic incr may not
      for (size_t i = 0; i < kIncrsPerThread; i++) {
        fetchAddU64S(boxPtr, 1);
      }
    }}});
  }
  for (auto &t: threads) {
    t->join();
  }
  uint64_t expected = kNumThreads * kIncrsPerThread;
  EXPECT_EQ(expected, loadU64S(box.ptr()));
}