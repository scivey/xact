#include <map>
#include <atomic>
#include <random>
#include <limits>
#include <unordered_map>
#include <glog/logging.h>
#include <benchmark/benchmark.h>
#include <sstream>
#include "xact/detail/backoff/XorShiftEngine.h"
#include "xact/detail/backoff/ExponentialBackoff.h"

using namespace xact::detail::backoff;

template<typename T,
  typename = typename std::enable_if<std::is_integral<T>::value, T>::type>
std::uniform_int_distribution<T> makeFullDist() {
  return std::uniform_int_distribution<T>{
    std::numeric_limits<T>::min(),
    std::numeric_limits<T>::max()
  };
}


static void BM_std_mt19937_engine(benchmark::State& state) {
  std::mt19937 engine {std::random_device()()};
  auto dist = makeFullDist<uint32_t>();
  while(state.KeepRunning()) {
    benchmark::DoNotOptimize(dist(engine));
  }
}
BENCHMARK(BM_std_mt19937_engine);


static void BM_xorshift_engine(benchmark::State& state) {
  XorShiftEngine engine {std::random_device()()};
  auto dist = makeFullDist<uint32_t>();
  while(state.KeepRunning()) {
    benchmark::DoNotOptimize(dist(engine));
  }
}
BENCHMARK(BM_xorshift_engine);


class NaiveBackoff {
 protected:
  std::mt19937 engine_;
  float start_ {0.f};
  float max_ {0.f};
  float current_ {0.f};
  float makeJitter(float num) {
    float maxJitter = std::min(num/2.f, 100.f);
    std::uniform_real_distribution<float> dist {1.f, maxJitter};
    return dist(engine_);
  }
 public:
  NaiveBackoff(float start, float lim, size_t seed = 0)
    : start_(start), max_(lim), current_(start_) {
    if (seed == 0) {
      seed = std::random_device()();
    }
    engine_.seed(seed);
    reset();
  }
  void reset() {
    current_ = start_ + makeJitter(start_);
  }
  float next() {
    float current = current_;
    if (current >= max_) {
      return current - makeJitter(current);
    }
    float next = std::pow(current, 2);
    next += makeJitter(next);
    if (next > max_) {
      next = max_;
    }
    current_ = next;
    return current;    
  }
};

static void BM_naive_backoff(benchmark::State& state) {
  NaiveBackoff backoff {0.f, 50000.f, 73};
  while(state.KeepRunning()) {
    for (size_t i = 0; i < 50; i++) {
      benchmark::DoNotOptimize(backoff.next());
    }
  }
}
BENCHMARK(BM_naive_backoff);


static void BM_xact_backoff(benchmark::State& state) {
  ExponentialBackoff backoff {0, 500000, 73};
  while(state.KeepRunning()) {
    for (size_t i = 0; i < 50; i++) {
      benchmark::DoNotOptimize(backoff.next());
    }
  }
}
BENCHMARK(BM_xact_backoff);

