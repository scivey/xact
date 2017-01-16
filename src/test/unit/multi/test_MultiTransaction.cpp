#include <gtest/gtest.h>
#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <vector>
#include <atomic>

#include "xact/TransactionExecutor.h"
#include "xact/multi/MultiTransaction.h"
#include "xact/detail/asm/util.h"
#include "xact/detail/macros.h"
#include "xact/LockableAtomicU64.h"
#include "xact/TransactionStatus.h"
#include "xact_testing/ThreadGroup.h"

using namespace std;
using xact::LockableAtomicU64;
using xact_testing::ThreadGroup;
using xact::detail::LockableAtomicU64Inspector;
using xact::TransactionStatus;
using xact::multi::MultiTransaction;

using namespace xact;

static xact_lockable_atomic_u64_t* getPointer(LockableAtomicU64& atom) {
  return (xact_lockable_atomic_u64_t*) LockableAtomicU64Inspector(atom).getPointer();
}
using gencas_exec_t = TransactionExecutor<DefaultTransactionRetryPolicy>;

using AU64 = xact::LockableAtomicU64;

using atom_t = xact_lockable_atomic_u64_t;
TEST(TestMultiTransaction, TestLoad2MVCC) {
  AU64 x{10}, y{20};
  uint64_t result1 {0}, result2 {0};
  gencas_exec_t executor;
  vector<pair<atom_t*, uint64_t*>> loadArgs {
    {getPointer(x), &result1},
    {getPointer(y), &result2}
  };
  auto trans = MultiTransaction::load(loadArgs);
  auto result = executor.execute(trans);
  EXPECT_EQ(TransactionStatus::OK, result);
  EXPECT_EQ(10, result1);
  EXPECT_EQ(20, result2);
}

TEST(TestMultiTransaction, TestStore2TSX) {
  AU64 x{0}, y{0};
  uint64_t xVal {100}, yVal {200};
  gencas_exec_t executor;
  vector<pair<atom_t*, uint64_t>> storeArgs {
    {getPointer(x), xVal},
    {getPointer(y), yVal}
  };
  EXPECT_EQ(
    TransactionStatus::OK,
    executor.execute(MultiTransaction::store(storeArgs))
  );
  uint64_t xResult {0}, yResult {0};
  vector<pair<atom_t*, uint64_t*>> loadArgs {
    {getPointer(x), &xResult},
    {getPointer(y), &yResult}
  };
  EXPECT_EQ(TransactionStatus::OK, executor.execute(MultiTransaction::load(loadArgs)));
  EXPECT_EQ(100, xResult);
  EXPECT_EQ(200, yResult);
}


TEST(TestMultiTransaction, TestStore2WithLocks) {
  AU64 x{0}, y{0};
  uint64_t xVal {100}, yVal {200};
  gencas_exec_t executor;
  vector<pair<atom_t*, uint64_t>> storeArgs {
    {getPointer(x), xVal},
    {getPointer(y), yVal}
  };
  {
    auto storeOp = MultiTransaction::store(storeArgs);
    EXPECT_EQ(TransactionStatus::OK, storeOp.lockAndExecute());
  }
  uint64_t xResult {0}, yResult {0};
  vector<pair<atom_t*, uint64_t*>> loadArgs {
    {getPointer(x), &xResult},
    {getPointer(y), &yResult}
  };
  EXPECT_EQ(TransactionStatus::OK, executor.execute(MultiTransaction::load(loadArgs)));
  EXPECT_EQ(100, xResult);
  EXPECT_EQ(200, yResult);
}


TEST(TestMultiTransaction, TestLoad2MVCC_MT) {
  AU64 x{0}, y{0};
  auto threads = ThreadGroup::createShared(4, [&x, &y](size_t) {
    gencas_exec_t executor;
    for (size_t i = 0; i < 10000; i++) {
      for (;;) {
        uint64_t xVal {0}, yVal {0};
        vector<pair<atom_t*, uint64_t*>> loadArgs {
          {getPointer(x), &xVal},
          {getPointer(y), &yVal}
        };
        auto result = executor.execute(MultiTransaction::load(loadArgs));
        if (result == TransactionStatus::OK) {
          break;
        }
      }
    }
  });
  threads->join();
}
