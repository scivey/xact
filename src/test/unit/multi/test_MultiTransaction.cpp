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

using namespace std;
using xact::LockableAtomicU64;
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

