#include <glog/logging.h>
#include <benchmark/benchmark.h>

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}
