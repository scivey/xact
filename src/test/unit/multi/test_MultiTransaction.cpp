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
using namespace xact;

static uint64_t* getPointer(LockableAtomicU64& atom) {
  return LockableAtomicU64Inspector(atom).getPointer();
}
using gencas_exec_t = TransactionExecutor<DefaultTransactionRetryPolicy>;

TEST(TestMultiTransaction, TestSanity1) {
  EXPECT_TRUE(true);
}

