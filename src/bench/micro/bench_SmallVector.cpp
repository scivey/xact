#include <glog/logging.h>
#include <benchmark/benchmark.h>
#include <algorithm>
#include <random>
#include <vector>
#include "xact/LockableAtomicU64.h"
#include "xact/TransactionExecutor.h"
#include "xact/TransactionStatus.h"
#include "xact/generalized_cas/GeneralizedCAS.h"
#include "xact/generalized_cas/ArrayStoragePolicy.h"
#include "xact/detail/asm/generalized_cas.h"
#include "xact/detail/SmallVector.h"

using namespace xact;
using namespace xact::detail;


static void BM_SmallVector_shuffle_and_sort(benchmark::State& state) {
  SmallVector<uint64_t> nums;
  for (size_t i = 0; i < 30; i++) {
    nums.push_back(i);
  }
  std::mt19937 engine {std::random_device()()};
  while(state.KeepRunning()) {
    std::shuffle(nums.begin(), nums.end(), engine);
    std::sort(nums.begin(), nums.end());
  }
}
BENCHMARK(BM_SmallVector_shuffle_and_sort);


static void BM_std_vector_shuffle_and_sort(benchmark::State& state) {
  std::vector<uint64_t> nums;
  nums.reserve(60);
  for (size_t i = 0; i < 30; i++) {
    nums.push_back(i);
  }
  std::mt19937 engine {std::random_device()()};
  while(state.KeepRunning()) {
    std::shuffle(nums.begin(), nums.end(), engine);
    std::sort(nums.begin(), nums.end());
  }
}
BENCHMARK(BM_std_vector_shuffle_and_sort);

