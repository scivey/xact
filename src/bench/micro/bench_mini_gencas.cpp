#include <map>
#include <atomic>

#include <unordered_map>
#include <glog/logging.h>
#include <benchmark/benchmark.h>
#include <sstream>
#include "xact/LockableAtomicU64.h"
#include "xact/TransactionExecutor.h"
#include "xact/TransactionStatus.h"
#include "xact/generalized_cas/GeneralizedCAS.h"
#include "xact/generalized_cas/ArrayStoragePolicy.h"
#include "xact/detail/asm/generalized_cas.h"

using namespace xact;
using namespace xact::generalized_cas;
using gencas_t = GeneralizedCAS<ArrayStoragePolicy<16>, ExceptionErrorPolicy>;
using gencas_exec_t = TransactionExecutor<DefaultTransactionRetryPolicy>;


static void BM_plain_atomic_cas(benchmark::State& state) {
  std::atomic<uint64_t> atom {0};
  while(state.KeepRunning()) {
    uint64_t current = atom.load();
    auto desired = current + 1;
    for (;;) {
      if (atom.compare_exchange_strong(current, desired)) {
        break;
      }
    }
  }
}
BENCHMARK(BM_plain_atomic_cas);

static void BM_plain_atomic_load(benchmark::State& state) {
  std::atomic<uint64_t> atom {0};
  while(state.KeepRunning()) {
    benchmark::DoNotOptimize(atom.load());
  }
}
BENCHMARK(BM_plain_atomic_load);

template<size_t N>
void do_gencas_multi_load() {
  std::array<LockableAtomicU64, N> atoms;
  std::array<uint64_t, N> result;
  gencas_t transaction;
  for (size_t i = 0; i < N; i++) {
    transaction.push(Operation::load(
      &atoms[0], &result[0]
    ));
  }
  gencas_exec_t executor;
  benchmark::DoNotOptimize(executor.execute(transaction));  
}

static void BM_generalized_cas_multi_load_4(benchmark::State& state) {
  while(state.KeepRunning()) {
    do_gencas_multi_load<4>();
  }
}
BENCHMARK(BM_generalized_cas_multi_load_4);

static void BM_generalized_cas_multi_load_8(benchmark::State& state) {
  while(state.KeepRunning()) {
    do_gencas_multi_load<8>();
  }
}
BENCHMARK(BM_generalized_cas_multi_load_8);


using gencas_exec_func_t =
  std::function<uint64_t(void*, uint64_t, void*, uint64_t)>;


template<size_t N = 8>
void doGencasWithoutTsx(gencas_exec_func_t func) {
  std::array<LockableAtomicU64, N> atoms;
  std::array<uint64_t, N> result;
  gencas_t transaction;
  for (size_t i = 0; i < N; i++) {
    transaction.push(Operation::load(
      &atoms[0], &result[0]
    ));
  }
  auto casOp = transaction.buildCASOpPrivate();
  auto& core = casOp.core();
  auto status = transactionStatusFromAbortCode(
    func(
      core.preconditions, core.nPreconditions,
      core.operations, core.nOperations
    )
  );
  CHECK(status == TransactionStatus::OK);
}


static void BM_generalized_cas_without_tsx_v1(benchmark::State& state) {
  while (state.KeepRunning()) {
    doGencasWithoutTsx(xact_generalized_cas_op_v1_with_locks_acquired);
  }
}

BENCHMARK(BM_generalized_cas_without_tsx_v1);


static void BM_generalized_cas_without_tsx_v2(benchmark::State& state) {
  while (state.KeepRunning()) {
    doGencasWithoutTsx(xact_generalized_cas_op_v2_with_locks_acquired);
  }
}

BENCHMARK(BM_generalized_cas_without_tsx_v2);
