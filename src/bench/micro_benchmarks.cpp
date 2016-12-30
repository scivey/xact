#include <map>
#include <atomic>

#include <unordered_map>
#include <glog/logging.h>
#include <benchmark/benchmark.h>
#include <sstream>
#include "xact/LockableAtomicU64.h"
#include "xact/generalized_cas/GeneralizedCAS.h"
#include "xact/generalized_cas/GeneralizedCASExecutor.h"
#include "xact/generalized_cas/ArrayStoragePolicy.h"

using xact::LockableAtomicU64;
using namespace xact::generalized_cas;

using gencas_t = GeneralizedCAS<ArrayStoragePolicy<16>, ExceptionErrorPolicy>;
using gencas_exec_t = GeneralizedCASExecutor<DefaultCASExecutorRetryPolicy>;

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

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}
