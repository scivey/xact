#include <random>
#include <functional>
#include <vector>
#include <atomic>
#include <thread>
#include <memory>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include "xact/detail/RWSeqLock.h"
#include "xact_testing/ThreadGroup.h"
using namespace std;
using namespace xact::detail;
using xact_testing::ThreadGroup;


TEST(TestRWSeqLock, TestLockUnlockRead) {
  RWSeqLock slock; 
  EXPECT_EQ(0, slock.getVersion());
  EXPECT_FALSE(slock.isLocked());
  EXPECT_FALSE(slock.isReadLocked());
  EXPECT_FALSE(slock.isWriteLocked());
  EXPECT_TRUE(slock.tryReadLock());
  EXPECT_TRUE(slock.isLocked());
  EXPECT_TRUE(slock.isReadLocked());
  EXPECT_FALSE(slock.isWriteLocked());
  EXPECT_EQ(0, slock.getVersion());
}


TEST(TestRWSeqLock, TestLockUnlockWrite1) {
  RWSeqLock slock;
  EXPECT_EQ(0, slock.getVersion());
  EXPECT_FALSE(slock.isLocked());
  EXPECT_FALSE(slock.isReadLocked());
  EXPECT_FALSE(slock.isWriteLocked());
  EXPECT_TRUE(slock.tryWriteLock());
  EXPECT_TRUE(slock.isLocked());
  EXPECT_TRUE(slock.isWriteLocked());
  EXPECT_FALSE(slock.isReadLocked());
  EXPECT_EQ(1, slock.getVersion());
}


TEST(TestRWSeqLock, TestLockUnlockWrite2) {
  RWSeqLock slock;
  EXPECT_EQ(0, slock.getVersion());
  EXPECT_TRUE(slock.tryWriteLock());
  EXPECT_EQ(1, slock.getVersion());
  EXPECT_FALSE(slock.tryWriteLock());
  EXPECT_FALSE(slock.tryReadLock());
  EXPECT_EQ(1, slock.getVersion());
  slock.writeUnlock();
  EXPECT_EQ(2, slock.getVersion());
  slock.writeLock();
  EXPECT_EQ(3, slock.getVersion());
  slock.writeUnlock();
  EXPECT_EQ(4, slock.getVersion());
}


TEST(TestRWSeqLock, TestStressST) {
  std::mt19937 engine {std::random_device()()};
  std::uniform_int_distribution<size_t> dist {0, 100};
  RWSeqLock slock;
  size_t expectedVersion = 0;
  for (size_t i = 0; i < 10000; i++) {
    auto whimsy = dist(engine);
    EXPECT_EQ(expectedVersion, slock.getVersion());
    if (whimsy % 2 == 0) {
      // write
      EXPECT_FALSE(slock.isLocked());
      EXPECT_FALSE(slock.isWriteLocked());
      EXPECT_FALSE(slock.isReadLocked());
      EXPECT_TRUE(slock.tryWriteLock());
      EXPECT_TRUE(slock.isWriteLocked());
      EXPECT_TRUE(slock.isLocked());
      EXPECT_FALSE(slock.isReadLocked());
      expectedVersion++;
      EXPECT_EQ(expectedVersion, slock.getVersion());
      slock.writeUnlock();
      expectedVersion++;
      EXPECT_EQ(expectedVersion, slock.getVersion());
      EXPECT_FALSE(slock.isLocked());
    } else {
      // read
      EXPECT_FALSE(slock.isLocked());
      EXPECT_FALSE(slock.isWriteLocked());
      EXPECT_FALSE(slock.isReadLocked());
      EXPECT_TRUE(slock.tryReadLock());
      EXPECT_TRUE(slock.isReadLocked());
      EXPECT_TRUE(slock.isLocked());
      EXPECT_FALSE(slock.isWriteLocked());

      // reads don't modify version
      EXPECT_EQ(expectedVersion, slock.getVersion());
      slock.readUnlock();
      EXPECT_EQ(expectedVersion, slock.getVersion());

      EXPECT_FALSE(slock.isLocked());
    }
  }
}

TEST(TestRWSeqLock, TestStressMT) {
  RWSeqLock slock;
  size_t expectedVersion = 0;
  static const size_t kNThreads = 8;
  static const size_t kNOpsPerThread = 10000;
  std::atomic<size_t> numAcquiredAtom {0};
  size_t numAcquiredBare {0};
  auto threads = ThreadGroup::createShared(kNThreads,
    [&slock, &numAcquiredAtom, &numAcquiredBare](size_t threadIdx) {
      const bool isWriter = threadIdx % 2 == 1;
      for (size_t i = 0; i < kNOpsPerThread; i++) {
        for (;;) {
          bool success {false};
          if (isWriter && slock.tryWriteLock()) {
            success = true;
            numAcquiredBare++;
            slock.writeUnlock();
          } else if (!isWriter && slock.tryReadLock()) {
            success = true;
            numAcquiredBare++;            
            slock.readUnlock();
          }
          if (success) {
            numAcquiredAtom.fetch_add(1);
            break;
          }
        }
      }
    }
  );
  threads->join();
  const size_t kExpectedAcquires = kNThreads * kNOpsPerThread;

  EXPECT_EQ(kExpectedAcquires, numAcquiredAtom.load());
  EXPECT_EQ(kExpectedAcquires, numAcquiredBare);
  const size_t kExpectedVersion = (kNThreads / 2) * kNOpsPerThread * 2;
  EXPECT_EQ(kExpectedVersion, slock.getVersion());
}
