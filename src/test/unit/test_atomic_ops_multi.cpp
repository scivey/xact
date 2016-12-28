#include <gtest/gtest.h>
#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <vector>
#include <atomic>
#include <array>

#include "xact/atomic_ops/single.h"
#include "xact/atomic_ops/multi.h"
#include "xact/atomic_ops/MultiOps.h"

#include "xact/AlignedBox.h"
#include "xact/AtomicU64.h"
#include "xact/fence.h"
#include "xact/util/util.h"

using namespace xact::atomic_ops;
using namespace std;
using xact::mFence;
using Box64 = xact::AtomicU64::box_t;

std::unique_ptr<Box64> heapBox() {
  return std::unique_ptr<Box64>{new Box64{0}};
}

using Mops = MultiOps<UniformMultiRetryPolicy<FixedNumRetries<100>, FixedNumRetries<10>>>;

template<typename TArray>
void zeroArray(TArray& anArray) {
  for (auto& elem: anArray) {
    elem = 0;
  }
}



TEST(TestU64MultiOps, TestSmokeStoreLoadSingleOnStack) {
  Box64 box {0};
  auto mops = Mops();
  auto bPtr = box.ptr();
  uint64_t dests[] = {0};
  uint64_t sources[] = {0};
  auto clearData = [&dests, &sources]() {
    dests[0] = 0;
    sources[0] = 0;
  };
  clearData();
  uint64_t* targets[] = {bPtr};
  const uint8_t nAtoms = 1;
  sources[0] = 10;
  EXPECT_TRUE(mops.storeU64M(targets, sources, nAtoms));
  EXPECT_TRUE(mops.loadU64M(targets, dests, nAtoms));
  EXPECT_EQ(10, dests[0]);
  clearData();
  storeU64S(targets[0], 20);
  EXPECT_TRUE(mops.loadU64M(targets, dests, nAtoms));
  EXPECT_EQ(20, dests[0]);
  clearData();
  sources[0] = 30;
  EXPECT_TRUE(mops.storeU64M(targets, sources, nAtoms));
  EXPECT_EQ(30, loadU64S(targets[0]));
}


TEST(TestU64MultiOps, TestSmokeStoreLoadMultiOnStack) {
  static const size_t kNumBoxes = 6;
  const uint64_t initVal = 13;
  array<Box64, kNumBoxes> boxes;
  array<uint64_t, kNumBoxes> sources;
  array<uint64_t, kNumBoxes> dests;
  array<uint64_t*, kNumBoxes> targets;
  for (size_t i = 0; i < kNumBoxes; i++) {
    boxes[i].value = initVal;
    targets[i] = boxes[i].ptr();
    sources[i] = 0;
    dests[i] = 0;
  }
  auto mops = Mops();
  const uint8_t kN = (uint8_t) kNumBoxes;
  EXPECT_TRUE(mops.loadU64M(targets, dests));
  for (auto elem: dests) {
    EXPECT_EQ(initVal, elem);
  }
  zeroArray(sources);
  zeroArray(dests);
  for (size_t i = 0; i < kNumBoxes; i++) {
    sources[i] = (i + 1) * 10;
  }
  EXPECT_TRUE(mops.storeU64M(targets, sources));
  EXPECT_TRUE(mops.loadU64M(targets, dests));
  for (size_t i = 0; i < kNumBoxes; i++) {
    auto expected = (i+1) * 10;
    EXPECT_EQ(expected, dests[i]);
  }
}

TEST(TestU64MultiOps, TestSmokeAddSingleOnStack) {
  const uint64_t initVal = 7;
  Box64 box {initVal};
  auto bPtr = box.ptr();
  uint64_t dests[] = {0};
  uint64_t sources[] = {0};
  auto clearData = [&dests, &sources]() {
    dests[0] = 0;
    sources[0] = 0;
  };
  clearData();
  uint64_t* targets[] = {bPtr};
  const uint8_t nAtoms = 1;
  auto mops = Mops();
  EXPECT_TRUE(mops.addU64M(targets, 10, nAtoms));
  EXPECT_TRUE(mops.loadU64M(targets, dests, nAtoms));
  EXPECT_EQ(initVal+10, dests[0]);
  EXPECT_TRUE(mops.addU64M(targets, 50, nAtoms));
  EXPECT_TRUE(mops.loadU64M(targets, dests, nAtoms));
  EXPECT_EQ(initVal+10+50, dests[0]);
}


TEST(TestU64MultiOps, TestSmokeAddMultiOnStack) {
  static const size_t kNumBoxes = 6;
  const uint64_t initVal = 10;
  array<Box64, kNumBoxes> boxes;
  array<uint64_t, kNumBoxes> sources;
  array<uint64_t, kNumBoxes> dests;
  array<uint64_t*, kNumBoxes> targets;
  for (size_t i = 0; i < kNumBoxes; i++) {
    boxes[i].value = initVal;
    targets[i] = boxes[i].ptr();
    sources[i] = 0;
    dests[i] = 0;
  }
  auto mops = Mops();
  const uint8_t kN = (uint8_t) kNumBoxes;
  EXPECT_TRUE(mops.loadU64M(targets, dests));
  for (auto elem: dests) {
    EXPECT_EQ(initVal, elem);
  }
  const uint64_t addBy = 7;
  EXPECT_TRUE(mops.addU64M(targets.data(), addBy, kN));
  zeroArray(dests);
  EXPECT_TRUE(mops.loadU64M(targets, dests));
  for (auto elem: dests) {
    EXPECT_EQ(initVal+addBy, elem);
  }
}

TEST(TestU64MultiOps, TestSmokeFetchAddFetchSubMultiOnStack) {
  static const size_t kNumBoxes = 6;
  const uint64_t initVal = 10;
  array<Box64, kNumBoxes> boxes;
  array<uint64_t, kNumBoxes> sources;
  array<uint64_t, kNumBoxes> dests;
  array<uint64_t*, kNumBoxes> targets;
  for (size_t i = 0; i < kNumBoxes; i++) {
    boxes[i].value = initVal;
    targets[i] = boxes[i].ptr();
    sources[i] = 0;
    dests[i] = 0;
  }
  auto mops = Mops();
  const uint8_t kN = (uint8_t) kNumBoxes;
  EXPECT_TRUE(mops.loadU64M(targets, dests));
  for (auto elem: dests) {
    EXPECT_EQ(initVal, elem);
  }
  const uint64_t addBy = 7;
  array<uint64_t, kNumBoxes> prevVals;
  EXPECT_TRUE(mops.fetchAddU64M(targets, prevVals, addBy));
  for (auto elem: prevVals) {
    EXPECT_EQ(initVal, elem);
  }
  zeroArray(dests);
  EXPECT_TRUE(mops.loadU64M(targets, dests));
  for (auto elem: dests) {
    EXPECT_EQ(initVal+addBy, elem);
  }
  EXPECT_TRUE(mops.fetchSubU64M(targets, prevVals, 2));
  for (auto elem: prevVals) {
    EXPECT_EQ(initVal+addBy, elem);
  }
  EXPECT_TRUE(mops.loadU64M(targets, dests));
  for (auto elem: dests) {
    EXPECT_EQ((initVal+addBy) - 2, elem);
  }  
}



TEST(TestU64MultiOps, TestSmokeCASSingleOnStack) {
  static const uint64_t kInitVal = 10;
  Box64 box {kInitVal};
  uint64_t* targets[] = {box.ptr()};
  uint64_t expected[] = {kInitVal};
  uint64_t desired[] = {105};
  auto mops = Mops();
  EXPECT_TRUE(mops.casU64M(targets, expected, desired, 1));
  EXPECT_EQ(105, box.value);
}


TEST(TestU64MultiOps, TestSmokeCASMultiOnStack) {
  static const size_t kNumBoxes = 10;
  static const uint64_t kInitVal = 10;
  static const uint8_t kN = (uint8_t) kNumBoxes;
  array<Box64, kNumBoxes> boxes;
  array<uint64_t*, kNumBoxes> targets;
  array<uint64_t, kNumBoxes> loaded;
  array<uint64_t, kNumBoxes> desired;
  zeroArray(loaded);
  zeroArray(desired);
  auto mops = Mops();
  for (size_t i = 0; i < kNumBoxes; i++) {
    boxes[i].value = (i + 1) * kInitVal;
    targets[i] = boxes[i].ptr();
  }
  array<uint64_t, kNumBoxes> expectedValues;
  EXPECT_TRUE(mops.loadU64M(targets, loaded));
  for (size_t i = 0; i < kNumBoxes; i++) {
    auto current = loaded[i];
    auto expected = (i+1) * kInitVal;
    EXPECT_EQ(expected, current);
    auto desiredVal = current + 100*(i+1);
    desired[i] = desiredVal;
    expectedValues[i] = desiredVal;
  }
  EXPECT_TRUE(mops.casU64M(targets, loaded, desired));
  zeroArray(loaded);
  zeroArray(desired);
  EXPECT_TRUE(mops.loadU64M(targets, loaded));
  for (size_t i = 0; i < kNumBoxes; i++) {
    auto expected = expectedValues[i];
    auto actual = loaded[i];
    EXPECT_EQ(expected, actual);
  }
}



TEST(TestU64MultiOps, TestReaderAdderConsistent) {
  static const size_t kNumBoxes = 10;
  static const size_t kNIncrements = 100000;
  static const uint64_t kInitVal = 0;
  static const uint64_t kIncrBy = 2;
  const uint64_t kExpectedFinalValue {
    kInitVal + kNIncrements * kIncrBy
  };
  array<Box64, kNumBoxes> boxes;
  array<uint64_t*, kNumBoxes> targets;
  for (size_t i = 0; i < kNumBoxes; i++) {
    boxes[i].value = kInitVal;
    targets[i] = boxes[i].ptr();
  }
  mFence();
  std::thread reader([&targets]() {
    const uint8_t kN = (uint8_t) kNumBoxes;
    array<uint64_t, kNumBoxes> loaded;
    zeroArray(loaded);
    for (;;) {
      if (loadU64M(targets, loaded)) {
        auto firstElem = loaded[0];
        for (auto elem: loaded) {
          EXPECT_EQ(firstElem, elem)
            << "multi-load's view of data should be a consistent snapshot.";
        }
        if (firstElem >= kExpectedFinalValue) {
          break;
        }
      }
    }
  });
  std::thread adder([&targets]() {
    size_t nSuccess = 0;
    const uint8_t kN = (uint8_t) kNumBoxes;
    auto mops = Mops();
    while (nSuccess < kNIncrements) {
      for (;;) {
        if (mops.addU64M(targets.data(), kIncrBy, kN)) {
          nSuccess++;
          break;
        }
      }
    }
  });
  reader.join();
  adder.join();
  std::array<uint64_t, kNumBoxes> finalData;
  const uint8_t kN = (uint8_t) kNumBoxes;
  EXPECT_TRUE(Mops().loadU64M(targets, finalData));
  for (auto val: finalData) {
    EXPECT_EQ(kExpectedFinalValue, val);
  }
}





TEST(TestU64MultiOps, TestThreadedMultiCAS) {
  static const size_t kNumBoxes = 8;
  static const size_t kIncrsPerThread = 50000;
  static const size_t kNWriters = 4;
  static const size_t kNReaders = 2;
  static const uint64_t kInitVal = 0;
  static const uint64_t kIncrBy = 1;
  static const uint8_t kN = (uint8_t) kNumBoxes;

  const uint64_t kExpectedFinalValue {
    kInitVal + (kIncrsPerThread * kIncrBy * kNWriters)
  };
  array<Box64, kNumBoxes> boxes;
  array<uint64_t*, kNumBoxes> targets;
  for (size_t i = 0; i < kNumBoxes; i++) {
    boxes[i].value = kInitVal;
    targets[i] = boxes[i].ptr();
  }
  mFence();
  std::vector<unique_ptr<thread>> threads;
  for (size_t i = 0; i < kNReaders; i++) {
    threads.push_back(std::unique_ptr<thread>{new thread{[&targets]() {
      auto mops = Mops();
      array<uint64_t, kNumBoxes> loaded;
      zeroArray(loaded);
      for (;;) {
        if (mops.loadU64M(targets, loaded)) {
          auto firstElem = loaded[0];
          for (auto elem: loaded) {
            EXPECT_EQ(firstElem, elem)
              << "multi-load's view of data should be a consistent snapshot.";
          }
          if (firstElem >= kExpectedFinalValue) {
            break;
          }
        }
      }      
    }}});
  }
  for (size_t i = 0; i < kNWriters; i++) {
    threads.push_back(std::unique_ptr<thread>{new thread{[&targets]() {
      array<uint64_t, kNumBoxes> loaded;
      array<uint64_t, kNumBoxes> desired;
      auto mops = Mops();
      size_t nSuccess = 0;
      while (nSuccess < kIncrsPerThread) {
        for (;;) {
          zeroArray(loaded);
          zeroArray(desired);
          if (mops.loadU64M(targets, loaded)) {
            for (size_t i = 0; i < kNumBoxes; i++) {
              desired[i] = loaded[i] + 1;
            }
            if (mops.casU64M(targets, loaded, desired)) {
              nSuccess++;
              break;
            }
          }
        }
      }
    }}});
  }
  for (auto& t: threads) {
    t->join();
  }
  array<uint64_t, kNumBoxes> loaded;
  zeroArray(loaded);
  auto mops = Mops();
  for (;;) {
    // I haven't actually hit a failure here, but I could see it being
    // possible if hardware implementation varies.
    // there's technically no guarantee a transaction will succeed,
    // even uncontended.
    if (mops.loadU64M(targets, loaded)) {
      break;
    }
  }
  for (auto elem: loaded) {
    EXPECT_EQ(kExpectedFinalValue, elem);
  }
}
