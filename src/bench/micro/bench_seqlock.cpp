#include <map>
#include <atomic>
#include <random>
#include <limits>
#include <unordered_map>
#include <glog/logging.h>
#include <benchmark/benchmark.h>
#include <sstream>
#include "xact/detail/asm/rw_seqlock.h"
#include "xact/detail/asm/rw_seqlock_ops.h"
#include "xact/detail/AlignedBox.h"
#include "xact/detail/RWSeqLock.h"

using namespace xact;
using namespace xact::detail;

using SlockBox = AlignedBox<xact_rw_seqlock_t, 16>;

static const size_t kIterations = 20;

static void BM_seqlock_read_lock_unlock(benchmark::State& state) {
  SlockBox slock;
  xact_rw_seqlock_init(slock.ptr());
  while(state.KeepRunning()) {
    for (size_t i = 0; i < kIterations; i++) {
      xact_rw_seqlock_read_lock(slock.ptr());
      xact_rw_seqlock_read_unlock(slock.ptr());      
    }
  }
}
BENCHMARK(BM_seqlock_read_lock_unlock);



static void BM_seqlock_cpp_read_lock_unlock(benchmark::State& state) {
  RWSeqLock slock;
  while(state.KeepRunning()) {
    for (size_t i = 0; i < kIterations; i++) {
      slock.readLock();
      slock.readUnlock();
    }
  }
}
BENCHMARK(BM_seqlock_cpp_read_lock_unlock);


static void BM_seqlock_is_read_locked(benchmark::State& state) {
  SlockBox slock;
  xact_rw_seqlock_init(slock.ptr());
  while(state.KeepRunning()) {
    for (size_t i = 0; i < kIterations; i++) {
      benchmark::DoNotOptimize(xact_rw_seqlock_is_read_locked(slock.ptr()));
    }
  }
}
BENCHMARK(BM_seqlock_is_read_locked);

static void BM_seqlock_cpp_is_read_locked(benchmark::State& state) {
  RWSeqLock slock;
  while(state.KeepRunning()) {
    for (size_t i = 0; i < kIterations; i++) {
      benchmark::DoNotOptimize(slock.isReadLocked());
    }
  }
}
BENCHMARK(BM_seqlock_cpp_is_read_locked);




static void BM_seqlock_get_version(benchmark::State& state) {
  SlockBox slock;
  xact_rw_seqlock_init(slock.ptr());
  while(state.KeepRunning()) {
    for (size_t i = 0; i < kIterations; i++) {
      benchmark::DoNotOptimize(xact_rw_seqlock_get_version(slock.ptr()));
    }
  }
}
BENCHMARK(BM_seqlock_get_version);

static void BM_seqlock_cpp_get_version(benchmark::State& state) {
  RWSeqLock slock;
  while(state.KeepRunning()) {
    for (size_t i = 0; i < kIterations; i++) {
      benchmark::DoNotOptimize(slock.getVersion());
    }
  }
}
BENCHMARK(BM_seqlock_cpp_get_version);




