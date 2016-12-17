#include <map>
#include <unordered_map>
#include <glog/logging.h>
#include <benchmark/benchmark.h>
#include <sstream>

static void BM_nothing(benchmark::State& state) {
  while (state.KeepRunning()) {
    size_t x = 0;
    for (size_t i = 0; i < 100; i++) {
      benchmark::DoNotOptimize(x = i);
    }

  }
}
BENCHMARK(BM_nothing);

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}
