#include <map>
#include <atomic>

#include <unordered_map>
#include <glog/logging.h>
#include <benchmark/benchmark.h>
#include <sstream>
#include "xact/AtomicU64.h"
#include "xact/FixedAtomicU64Group.h"

using xact::AtomicU64;
using AtomGroup2 = xact::FixedAtomicU64Group<2>;


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
struct CASBenchData {  
  std::array<AtomicU64, N> atoms;
  std::array<AtomicU64*, N> atomPtrs;
  std::array<uint64_t, N> expected;
  std::array<uint64_t, N> desired;
  CASBenchData() {
    for (size_t i = 0; i < N; i++) {
      atoms[i].store(0);
      atomPtrs[i] = atoms.data() + i;
    }
  }
};

template<size_t N>
struct GoodCASBenchContext {
  CASBenchData<N> data;
  xact::FixedAtomicU64Group<N> atomGroup;
  GoodCASBenchContext(): atomGroup(data.atomPtrs) {}
  void step() {
    for (;;) {
      if (atomGroup.load(data.expected)) {
        break;
      }
    }
    for (size_t i = 0; i < N; i++) {
      data.desired[i] = data.expected[i] + 1;
    }
    for (;;) {
      if (atomGroup.compareExchange(data.expected, data.desired)) {
        break;
      }
    }    
  }
};


template<size_t N>
struct BadCASBenchContext {
  CASBenchData<N> data;
  xact::FixedAtomicU64Group<N> atomGroup;
  BadCASBenchContext(): atomGroup(data.atomPtrs) {}
  void step() {
    for (;;) {
      if (atomGroup.load(data.expected)) {
        break;
      }
    }
    for (size_t i = 0; i < N; i++) {
      data.desired[i] = data.expected[i] + 1;
      data.expected[i] += 10;
    }
    atomGroup.compareExchange(data.expected, data.desired);
  }
};

static void BM_cas2_success_uncontended(benchmark::State& state) {
  GoodCASBenchContext<2> ctx;
  while (state.KeepRunning()) {
    ctx.step();
  }
}

BENCHMARK(BM_cas2_success_uncontended);

static void BM_cas2_failure_uncontended(benchmark::State& state) {
  BadCASBenchContext<2> ctx;
  while (state.KeepRunning()) {
    ctx.step();
  }
}
BENCHMARK(BM_cas2_failure_uncontended);


static void BM_load2_uncontended(benchmark::State& state) {
  GoodCASBenchContext<2> ctx;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(ctx.atomGroup.load(ctx.data.expected));
  }
}
BENCHMARK(BM_load2_uncontended);


static void BM_store2_uncontended(benchmark::State& state) {
  GoodCASBenchContext<2> ctx;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(ctx.atomGroup.store(ctx.data.expected));
  }
}
BENCHMARK(BM_store2_uncontended);

static void BM_add2_uncontended(benchmark::State& state) {
  GoodCASBenchContext<2> ctx;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(ctx.atomGroup.add(2));
  }
}
BENCHMARK(BM_add2_uncontended);

static void BM_cas8_success_uncontended(benchmark::State& state) {
  GoodCASBenchContext<8> ctx;
  while (state.KeepRunning()) {
    ctx.step();
  }
}
BENCHMARK(BM_cas8_success_uncontended);

static void BM_cas8_failure_uncontended(benchmark::State& state) {
  BadCASBenchContext<8> ctx;
  while (state.KeepRunning()) {
    ctx.step();
  }
}
BENCHMARK(BM_cas8_failure_uncontended);

static void BM_load8_uncontended(benchmark::State& state) {
  GoodCASBenchContext<8> ctx;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(ctx.atomGroup.load(ctx.data.expected));
  }
}
BENCHMARK(BM_load8_uncontended);

static void BM_store8_uncontended(benchmark::State& state) {
  GoodCASBenchContext<8> ctx;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(ctx.atomGroup.store(ctx.data.expected));
  }
}
BENCHMARK(BM_store8_uncontended);

static void BM_add8_uncontended(benchmark::State& state) {
  GoodCASBenchContext<8> ctx;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(ctx.atomGroup.add(1));
  }
}
BENCHMARK(BM_add8_uncontended);

static void BM_cas16_success_uncontended(benchmark::State& state) {
  GoodCASBenchContext<16> ctx;
  while (state.KeepRunning()) {
    ctx.step();
  }
}
BENCHMARK(BM_cas16_success_uncontended);


static void BM_cas16_failure_uncontended(benchmark::State& state) {
  BadCASBenchContext<16> ctx;
  while (state.KeepRunning()) {
    ctx.step();
  }
}
BENCHMARK(BM_cas16_failure_uncontended);


int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}
