#include <map>
#include <array>
#include <glog/logging.h>
#include <chrono>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>

#include "xact/util/PThreadSpinLock.h"
#include "xact/AtomicU64.h"
#include "xact/FixedAtomicU64Group.h"
#include "xact/macros.h"
#include "xact/fence.h"
#include "xact/AlignedBox.h"

using namespace std;
using xact::util::PThreadSpinLock;


template<typename T, size_t N, typename LockType>
class LockedNumArray {
  static_assert(
    std::is_integral<T>::value,
    "LockedNumArray expects an integral type."
  );
 public:
  using val_t = T;
  using data_t = std::array<uint64_t, N>;
  using boxed_data_t = std::array<xact::AlignedBox<uint64_t, 16>, N>;
  using lock_t = LockType;
 protected:
  boxed_data_t data_;
  lock_t& lock_;
 public:
  LockedNumArray(){}
  LockedNumArray(lock_t& lock): lock_(lock){}
  void store(const data_t& toStore) {
    std::lock_guard<lock_t> guard;
    for (size_t i = 0; i < N; i++) {
      data_[i] = toStore[i];
    }
  }
  void storeAt(size_t idx, T value) {
    std::lock_guard<lock_t> guard {lock_};
    data_[idx] = value;
  }
  void load(data_t& result) {
    std::lock_guard<lock_t> guard {lock_};
    for (size_t i = 0; i < N; i++) {
      result[i] = data_[i];
    }
  }
  void add(T value) {
    std::lock_guard<lock_t> guard {lock_};
    for (size_t i = 0; i < N; i++) {
      data_[i] += value;
    }
  }
};


template<size_t NAtoms, size_t NWriters, size_t NReaders, size_t IncrPerThread, size_t ReadsPerThread>
struct BenchParams {
  static const size_t kInitVal = 10;
  static const size_t kNAtoms = NAtoms;
  static const size_t kNWriters = NWriters;
  static const size_t kNReaders = NReaders;
  static const size_t kIncrPerThread = IncrPerThread;
  static const size_t kReadsPerThread = ReadsPerThread;
  static const size_t kExpectedFinalVal = kInitVal + kNWriters * kIncrPerThread;
};


template<typename TLock, typename BParams>
void runLockedSmart(TLock& mut) {
  using data_t = LockedNumArray<uint64_t, BParams::kNAtoms, TLock>;
  data_t atoms{mut};
  for (size_t i = 0; i < BParams::kNAtoms; i++) {
    atoms.storeAt(i, BParams::kInitVal);
  }
  vector<unique_ptr<thread>> threads;
  for (size_t i = 0; i < BParams::kNReaders; i++) {
    threads.push_back(std::unique_ptr<thread>{new thread{[&atoms]() {
      array<uint64_t, BParams::kNAtoms> loaded;
      for (size_t i = 0; i < BParams::kReadsPerThread; i++) {
        atoms.load(loaded);
        auto first = loaded[0];
        for (auto elem: loaded) {
          CHECK_EQ(first, elem);
        }
      }
    }}});
  }
  for (size_t i = 0; i < BParams::kNWriters; i++) {
    threads.push_back(std::unique_ptr<thread>{new thread{[&atoms]() {
      for (size_t i = 0; i < BParams::kIncrPerThread; i++) {
        atoms.add(1);
      }
    }}});
  }
  for (auto& t: threads) {
    t->join();
  }
}

template<typename BParams>
void runTransactional() {
  using AtomGroup = xact::FixedAtomicU64Group<BParams::kNAtoms>;
  array<xact::AtomicU64, BParams::kNAtoms> atoms;
  array<xact::AtomicU64*, BParams::kNAtoms> atomsPtrs;
  static const size_t NAtoms = BParams::kNAtoms;
  for (size_t i = 0; i < NAtoms; i++) {
    atoms[i].store(BParams::kInitVal);
    atomsPtrs[i] = atoms.data() + i;
  }
  vector<unique_ptr<thread>> threads;
  for (size_t i = 0; i < BParams::kNReaders; i++) {
    threads.push_back(std::unique_ptr<thread>{new thread{[&atomsPtrs]() {
      array<uint64_t, NAtoms> loaded;
      AtomGroup group {atomsPtrs};
      for (size_t i = 0; i < BParams::kReadsPerThread; i++) {
        for (;;) {
          if (group.load(loaded)) {
            auto first = loaded[0];
            for (auto elem: loaded) {
              CHECK_EQ(first, elem);
            }
            break;
          }
        }
      }
    }}});
  }
  for (size_t i = 0; i < BParams::kNWriters; i++) {
    threads.push_back(std::unique_ptr<thread>{new thread{[&atomsPtrs]() {
      array<uint64_t, NAtoms> expected;
      array<uint64_t, NAtoms> desired;
      AtomGroup group {atomsPtrs};
      for (size_t i = 0; i < BParams::kIncrPerThread; i++) {
        for (;;) {
          if (group.add(1)) {
            break;
          }
        }
      }
    }}});
  }
  for (auto &t: threads) {
    t->join();
  }
  AtomGroup group {atomsPtrs};
  array<uint64_t, NAtoms> loaded;
  for (;;) {
    if (group.load(loaded)) {
      break;
    }
  }
  auto first = loaded[0];
  CHECK_EQ(first, BParams::kExpectedFinalVal);
  for (auto elem: loaded) {
    CHECK_EQ(first, elem);
  }
}

class Timer {
  using sys_time_t = decltype(std::chrono::system_clock::now());
  sys_time_t startTime_;
  sys_time_t endTime_;
  bool running_ {false};
  bool ran_ {false};
 public:
  void start() {
    XACT_CHECK(!running_);
    startTime_ = std::chrono::system_clock::now();
    running_ = true;
    ran_ = false;
  }
  void stop() {
    XACT_CHECK(running_);
    endTime_ = std::chrono::system_clock::now();
    running_ = false;
    ran_ = true;
  }
  size_t elapsedNsec() {
    XACT_CHECK(running_ || ran_);
    if (running_) {
      return (std::chrono::system_clock::now() - startTime_).count();
    }
    return (endTime_ - startTime_).count();
  }
  double elapsedMsec() {
    double ns = elapsedNsec();
    return ns / 1000000.0;
  }
};


std::string leftPad(const std::string &strung, size_t N) {
  if (strung.size() > N) {
    return strung;
  }
  std::ostringstream oss;
  auto delta = N - strung.size();
  for (size_t i = 0; i < delta; i++) {
    oss << ' ';
  }
  oss << strung;
  return oss.str();
}

std::string leftPad(size_t val, size_t N) {
  std::ostringstream oss;
  oss << val;
  return leftPad(oss.str(), N);
}

std::string rightPad(const std::string &strung, size_t N) {
  if (strung.size() > N) {
    return strung;
  }
  std::ostringstream oss;
  oss << strung;
  auto delta = N - strung.size();
  for (size_t i = 0; i < delta; i++) {
    oss << ' ';
  }
  return oss.str();
}

template<typename BParams>
void runBattery() {
  cout << endl;
  cout << "\tatoms=" << BParams::kNAtoms << "  \treaders=" << BParams::kNReaders
       << "\twriters=" << BParams::kNWriters << "\tincrPerThread=" << BParams::kIncrPerThread;
  cout << endl;

  Timer timer;
  static const size_t kTrials = 5;
  {
    uint64_t elapsed {0};
    auto slock = PThreadSpinLock::create();
    for (size_t i = 0; i < kTrials; i++) {
      timer.start();
      runLockedSmart<decltype(slock), BParams>(slock);
      timer.stop();
      elapsed += timer.elapsedNsec();
    }
    elapsed /= kTrials;
    cout << "\t" << rightPad("spinlock elapsed: ", 40) << leftPad(elapsed, 16) << endl;
    // cout << "\tspinlock elapsed:              " << elapsed << endl;

  }  
  {
    uint64_t elapsed {0};
    for (size_t i = 0; i < kTrials; i++) {
      timer.start();
      std::mutex mut;
      runLockedSmart<std::mutex, BParams>(mut);
      timer.stop();
      elapsed += timer.elapsedNsec();
    }
    elapsed /= kTrials;
    cout << "\t" << rightPad("mutex elapsed: ", 40) << leftPad(elapsed, 16) << endl;
  }
  {
    uint64_t elapsed {0};
    for (size_t i = 0; i < kTrials; i++) {
      timer.start();
      runTransactional<BParams>();
      timer.stop();
      elapsed += timer.elapsedNsec();
    }
    elapsed /= kTrials;
    cout << "\t" << rightPad("transactional elapsed: ", 40) << leftPad(elapsed, 16) << endl;
  }
}


int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  static const size_t kDefaultWrites = 50000;
  runBattery<BenchParams<4, 1, 4, kDefaultWrites, kDefaultWrites>>();
  runBattery<BenchParams<4, 2, 4, kDefaultWrites, kDefaultWrites>>();
  runBattery<BenchParams<2, 8, 4, kDefaultWrites, kDefaultWrites>>();
  runBattery<BenchParams<2, 4, 4, kDefaultWrites, kDefaultWrites>>();
  runBattery<BenchParams<2, 1, 16, kDefaultWrites, kDefaultWrites>>();
  runBattery<BenchParams<1, 4, 4, kDefaultWrites, kDefaultWrites>>();

  runBattery<BenchParams<8, 1, 4, kDefaultWrites, kDefaultWrites>>();
  runBattery<BenchParams<8, 2, 4, kDefaultWrites, kDefaultWrites>>();
  runBattery<BenchParams<8, 1, 4, kDefaultWrites, kDefaultWrites>>();
  runBattery<BenchParams<8, 2, 4, kDefaultWrites, kDefaultWrites>>();

  runBattery<BenchParams<8, 1, 16, kDefaultWrites, kDefaultWrites>>();
  runBattery<BenchParams<8, 1, 8, kDefaultWrites, kDefaultWrites>>();

  runBattery<BenchParams<8, 4, 4, kDefaultWrites, kDefaultWrites>>();

  LOG(INFO) << "end";
}
