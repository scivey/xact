#include <map>
#include <atomic>

#include <unordered_map>
#include <glog/logging.h>
#include <benchmark/benchmark.h>
#include <sstream>
#include "xact/LockableAtomicU64.h"
#include "xact/TransactionExecutor.h"
#include "xact/TransactionStatus.h"

using namespace xact;
using gencas_exec_t = TransactionExecutor<DefaultTransactionRetryPolicy>;

static void BM_std_atomic_u64_load_single(benchmark::State& state) {
  std::atomic<uint64_t> atom {0};
  while(state.KeepRunning()) {
    benchmark::DoNotOptimize(atom.load());
  }
}
BENCHMARK(BM_std_atomic_u64_load_single);


static void BM_xact_atomic_u64_load_single(benchmark::State& state) {
  LockableAtomicU64 atom {0};
  gencas_exec_t executor;
  while(state.KeepRunning()) {
    uint64_t result {0};
    for (;;) {
      auto loadOp = atom.makeLoad(&result);
      auto outcome = executor.execute(loadOp);
      if (outcome == TransactionStatus::OK) {
        break;
      }
    }
  }
}
BENCHMARK(BM_xact_atomic_u64_load_single);


static void BM_std_atomic_u64_cas_single(benchmark::State& state) {
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
BENCHMARK(BM_std_atomic_u64_cas_single);

static void BM_xact_atomic_u64_cas_single(benchmark::State& state) {
  uint64_t baseExpected {0};
  LockableAtomicU64 atom {baseExpected};
  gencas_exec_t executor;
  while(state.KeepRunning()) {
    uint64_t desired {baseExpected + 1};
    for (;;) {
      uint64_t expected = baseExpected;
      auto casOp = atom.makeCompareExchange(&expected, desired);
      auto outcome = executor.execute(casOp);
      if (outcome == TransactionStatus::OK) {
        baseExpected = desired;
        break;
      }
    }
  }
}
BENCHMARK(BM_xact_atomic_u64_cas_single);


static void BM_std_atomic_u64_store_single(benchmark::State& state) {
  std::atomic<uint64_t> atom {0};
  while(state.KeepRunning()) {
    benchmark::DoNotOptimize(atom.load());
  }
}
BENCHMARK(BM_std_atomic_u64_store_single);


static void BM_xact_atomic_u64_store_single(benchmark::State& state) {
  LockableAtomicU64 atom {0};
  gencas_exec_t executor;
  while(state.KeepRunning()) {
    uint64_t arg {10};
    for (;;) {
      auto storeOp = atom.makeStore(arg);
      auto outcome = executor.execute(storeOp);
      if (outcome == TransactionStatus::OK) {
        break;
      }
    }
  }
}
BENCHMARK(BM_xact_atomic_u64_store_single);


static void BM_std_atomic_u64_fetch_add_single(benchmark::State& state) {
  std::atomic<uint64_t> atom {0};
  while(state.KeepRunning()) {
    benchmark::DoNotOptimize(atom.fetch_add(1));
  }
}
BENCHMARK(BM_std_atomic_u64_fetch_add_single);


static void BM_xact_atomic_u64_fetch_add_single(benchmark::State& state) {
  LockableAtomicU64 atom {0};
  gencas_exec_t executor;
  while(state.KeepRunning()) {
    uint64_t arg {10};
    uint64_t dest {0};
    for (;;) {
      auto fetchOp = atom.makeFetchAdd(&arg, dest);
      auto outcome = executor.execute(fetchOp);
      if (outcome == TransactionStatus::OK) {
        break;
      }
    }
  }
}
BENCHMARK(BM_xact_atomic_u64_fetch_add_single);
