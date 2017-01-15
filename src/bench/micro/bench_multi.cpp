#include <map>
#include <atomic>

#include <unordered_map>
#include <glog/logging.h>
#include <benchmark/benchmark.h>
#include <sstream>
#include "xact/LockableAtomicU64.h"
#include "xact/TransactionExecutor.h"
#include "xact/TransactionStatus.h"
#include "xact/multi/MultiTransaction.h"
#include "xact/detail/SmallVector.h"
using namespace xact;
using namespace xact::multi;
using namespace xact::detail;
using gencas_exec_t = TransactionExecutor<DefaultTransactionRetryPolicy>;
using LU64 = LockableAtomicU64;
using atom_t = xact_lockable_atomic_u64_t;
static atom_t* getPointer(LockableAtomicU64& atom) {
  return (atom_t*) LockableAtomicU64Inspector(atom).getPointer();
}

static void BM_xact_atomic_u64_load_2(benchmark::State& state) {
  LU64 x {12}, y {33};
  gencas_exec_t executor;
  while(state.KeepRunning()) {
    uint64_t result1 {0};
    uint64_t result2 {0};
    SmallVector<std::pair<atom_t*, uint64_t*>> args {
      {getPointer(x), &result1},
      {getPointer(y), &result2}
    };
    auto loadOp = MultiTransaction::load(args);
    auto outcome = executor.execute(loadOp);
    CHECK(outcome == TransactionStatus::OK);
    CHECK(result1 == 12);
    CHECK(result2 == 33);
  }
}
BENCHMARK(BM_xact_atomic_u64_load_2);

