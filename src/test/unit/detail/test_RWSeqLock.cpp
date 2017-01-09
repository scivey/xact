#include <random>
#include <gtest/gtest.h>
#include "xact/detail/RWSeqLock.h"

using namespace std;
using namespace xact::detail;


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
