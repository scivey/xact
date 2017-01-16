#include <gtest/gtest.h>
#include <string>
#include <functional>
#include <thread>
#include <chrono>

#include <atomic>
#include <memory>
#include <vector>
#include <atomic>
#include <glog/logging.h>

#include "xact/generalized_cas/GeneralizedCAS.h"
#include "xact/generalized_cas/Operation.h"
#include "xact/generalized_cas/Precondition.h"
#include "xact/detail/asm/util.h"
#include "xact/detail/asm/lockable_atomic_u64_ops.h"
#include "xact/detail/macros.h"

#include "xact/LockableAtomicU64.h"
#include "xact/TransactionStatus.h"
#include "xact/TransactionExecutor.h"
#include "xact_testing/ThreadGroup.h"

using namespace std;
using namespace xact;
using xact_testing::ThreadGroup;
using xact::detail::LockableAtomicU64Inspector;
using xact::LockableAtomicU64;
using xact::TransactionStatus;
using namespace xact::generalized_cas;
using gencas_exec_t = TransactionExecutor<DefaultTransactionRetryPolicy>;

uint64_t* valPtr(LockableAtomicU64 *target) {
  return LockableAtomicU64Inspector(*target).getPointer();
}



TEST(TestLockableAtomicU64, TestLoadStoreST) {
  gencas_exec_t exc;
  LockableAtomicU64 atom {0};
  uint64_t value = 0;
  CHECK(exc.execute(atom.makeLoad(&value)) == TransactionStatus::OK);
  EXPECT_EQ(0, value);
  value = 7;
  CHECK(exc.execute(atom.makeStore(value)) == TransactionStatus::OK);
  value = 0;
  CHECK(exc.execute(atom.makeLoad(&value)) == TransactionStatus::OK);
  XACT_MFENCE_BARRIER();
  EXPECT_EQ(7, value);


  value = 100;
  CHECK(exc.execute(atom.makeStore(value)) == TransactionStatus::OK);
  value = 0;
  CHECK(exc.execute(atom.makeLoad(&value)) == TransactionStatus::OK);
  EXPECT_EQ(100, value);
}

TEST(TestLockableAtomicU64, TestSingleThreadedCAS) {
  LockableAtomicU64 atom {0};
  uint64_t value = 0;
  CHECK(atom.store(value) == TransactionStatus::OK);
  size_t nSuccess {0};
  uint64_t previous = 0;
  gencas_exec_t executor;
  const size_t kIterations = 50000;
  while (nSuccess < kIterations) {    
    for (;;) {
      uint64_t expected = 0;
      TransactionStatus res = TransactionStatus::OK;
      for (;;) {
        auto loadOp = atom.makeLoad(&expected);
        auto result = executor.execute(loadOp);
        if (result == TransactionStatus::TSX_RETRY || res == TransactionStatus::TSX_CONFLICT) {
          continue;
        }
        break;
      }
      EXPECT_EQ(expected, previous);
      uint64_t desired = expected + 1;
      res = executor.execute(atom.makeCompareExchange(&expected, desired));
      if (res == TransactionStatus::OK) {
        previous = desired;
        nSuccess++;
        break;
      }
    }
  }
  uint64_t result;
  auto status = executor.execute(atom.makeLoad(&result));
  EXPECT_EQ(TransactionStatus::OK, status);
  EXPECT_EQ(kIterations, result);
}


TEST(TestLockableAtomicU64, TestSTSingleCASFailure) {
  LockableAtomicU64 atom {0};
  uint64_t value = 0;
  uint64_t expected {7};
  gencas_exec_t executor;
  auto op = atom.makeCompareExchange(&expected, value);
  EXPECT_EQ(TransactionStatus::PRECONDITION_FAILED, op.execute());
  EXPECT_EQ(0, expected);
  EXPECT_EQ(TransactionStatus::OK, atom.makeStore(50).execute());
  expected = 26;
  value = 10;
  auto op2 = atom.makeCompareExchange(&expected, value);
  EXPECT_EQ(TransactionStatus::PRECONDITION_FAILED, op2.execute());
  EXPECT_EQ(50, expected);
  expected = 10;
  EXPECT_EQ(TransactionStatus::OK, atom.makeLoad(&expected).execute());
  EXPECT_EQ(50, expected);
  value = 51;
  EXPECT_EQ(TransactionStatus::OK, atom.makeCompareExchange(&expected, value).execute());
  EXPECT_EQ(50, expected);
  EXPECT_EQ(51, value);
  value = 0;
  EXPECT_EQ(TransactionStatus::OK, atom.makeLoad(&value).execute());
  EXPECT_EQ(51, value);
}




TEST(TestLockableAtomicU64, TestMultithreadedStupidCAS) {
  LockableAtomicU64 atom {0};
  static const size_t kIncrPerThread = 50000;
  static const size_t kNThreads = 1;
  auto threads = ThreadGroup::createShared(kNThreads, [&atom](size_t) {
    size_t nSuccess = 0;
    gencas_exec_t executor;
    while (nSuccess < kIncrPerThread) {
      uint64_t expected = 0;
      for (;;) {
        auto res = atom.makeLoad(&expected).execute();
        EXPECT_EQ(TransactionStatus::OK, res);
        uint64_t desired = expected + 1;
        auto casOp = atom.makeCompareExchange(&expected, desired);
        res = casOp.execute();
        if (res == TransactionStatus::OK) {
          nSuccess++;
          break;
        }
      }
    }
  });
  threads->join();
  uint64_t result {0};
  EXPECT_EQ(atom.load(&result), TransactionStatus::OK);
  EXPECT_EQ(kNThreads*kIncrPerThread, result);
}

