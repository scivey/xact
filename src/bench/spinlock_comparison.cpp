#include <map>
#include <array>
#include <glog/logging.h>
#include <chrono>
#include <memory>
#include <thread>
#include <mutex>
#include <set>

#include <iostream>
#include <random>

#include "xact/detail/util/PThreadSpinLock.h"
#include "xact/LockableAtomicU64.h"
#include "xact/generalized_cas/GeneralizedCAS.h"
#include "xact/generalized_cas/ArrayStoragePolicy.h"
#include "xact/TransactionExecutor.h"
#include "xact/detail/macros.h"
#include "xact/detail/fence.h"
#include "xact/detail/AlignedBox.h"

using namespace std;
using xact::detail::util::PThreadSpinLock;
using xact::TransactionStatus;
using xact::TransactionExecutor;
using xact::DefaultTransactionRetryPolicy;

template<size_t N, typename LockType>
class LockedNumArray {
 public:
  using val_t = uint64_t;
  using boxed_data_t = std::array<xact::detail::AlignedBox<uint64_t, 16>, N>;
  using lock_t = LockType;
 protected:
  boxed_data_t data_;
  lock_t& lock_;
 public:
  LockedNumArray(){}
  LockedNumArray(lock_t& lock): lock_(lock){}
  static size_t maxSize() {
    return N;
  }
 protected:
  void writeNAt(std::pair<size_t, val_t> *pairs, size_t nPairs) {
    std::lock_guard<lock_t> guard {lock_};
    for (size_t i = 0; i < nPairs; i++) {
      std::pair<size_t, val_t> current = *pairs;
      data_[current.first] = current.second;
    }
  }
  void readNAt(size_t *idxs, size_t nIdxs, uint64_t *result) {
    std::lock_guard<lock_t> guard {lock_};
    for (size_t i = 0; i < nIdxs; i++) {
      size_t idx = *idxs;
      *result = data_[idx];
      ++idxs;
      ++result;
    }
  }
 public:
  void writeAt(size_t idx, val_t val) {
    std::lock_guard<lock_t> guard {lock_};
    data_[idx] = val;
  }
  template<size_t NElem>
  void writeNAt(std::array<std::pair<size_t, val_t>, NElem> &pairs) {
    writeNAt(pairs.data(), pairs.max_size());
  }

  val_t readAt(size_t idx) {
    std::lock_guard<lock_t> guard {lock_};
    return data_[idx];
  }

  template<size_t NElem>
  void readNAt(std::array<size_t, NElem> &idxs, std::array<val_t, NElem> &vals) {
    readNAt(idxs.data(), idxs.max_size(), vals.data());
  }

  template<size_t NElem>
  std::array<val_t, NElem> readNAt(std::array<size_t, NElem> &idxs) {
    std::array<val_t, NElem> result;
    readNAt(idxs, result);
    return result;
  }
};



template<size_t N>
class TransactionalNumArray {
 public:
  using val_t = uint64_t;
  using boxed_data_t = std::array<xact::LockableAtomicU64, N>;

  // using boxed_data_t = std::array<xact::detail::AlignedBox<
  //   xact::LockableAtomicU64, 64
  // >, N>;
  using Operation = xact::generalized_cas::Operation;
  using gencas_t = xact::generalized_cas::GeneralizedCAS<
    xact::generalized_cas::ArrayStoragePolicy<16>,
    xact::generalized_cas::ExceptionErrorPolicy
  >;
  using gencas_exec_t = xact::TransactionExecutor<
    xact::DefaultTransactionRetryPolicy
  >;
 protected:
  boxed_data_t data_;
  static thread_local gencas_exec_t casExecutor_;
 public:
  static size_t maxSize() {
    return N;
  }
  TransactionalNumArray() {
    gencas_exec_t executor;
    for (size_t i = 0; i < N; i++) {
      auto result = executor.execute(data_[i].makeStore(0));
      CHECK(result == TransactionStatus::OK)
        << "failed TransactionStatus: " << static_cast<uint64_t>(result);
    }
  }
 protected:
  void writeNAt(std::pair<size_t, val_t> *pairs, size_t nPairs) {
    gencas_t transaction;
    for (size_t i = 0; i < nPairs; i++) {
      std::pair<size_t, val_t> current = *pairs;
      ++pairs;
      transaction.push(
        Operation::store(&data_[current.first], current.second)
      );
    }
    TransactionStatus result = casExecutor_.execute(transaction);
    CHECK(result == TransactionStatus::OK)
      << "failed TransactionStatus: " << static_cast<uint64_t>(result);
  }
  void readNAt(size_t *idxs, size_t nIdxs, uint64_t *result) {
    gencas_t transaction;
    for (size_t i = 0; i < nIdxs; i++) {
      size_t idx = *idxs;
      transaction.push(
        Operation::load(&data_[idx], result)
      );
      ++idxs;
      ++result;
    }
    CHECK(casExecutor_.execute(transaction) == TransactionStatus::OK);
  }
 public:
  void writeAt(size_t idx, val_t val) {
    gencas_t transaction {
      {},
      {
        Operation::store(&data_[idx], val)
      }
    };
    CHECK(casExecutor_.execute(transaction) == TransactionStatus::OK);
  }

  template<size_t NElem>
  void writeNAt(std::array<std::pair<size_t, val_t>, NElem> &pairs) {
    writeNAt(pairs.data(), pairs.max_size());
  }

  val_t readAt(size_t idx) {
    val_t result;
    gencas_t transaction {
      {},
      {
        Operation::load(&data_[idx], &result)
      }
    };
    CHECK(casExecutor_.execute(transaction) == TransactionStatus::OK);
    return result;
  }

  template<size_t NElem>
  void readNAt(std::array<size_t, NElem> &idxs, std::array<val_t, NElem> &vals) {
    readNAt(idxs.data(), idxs.max_size(), vals.data());
  }

  template<size_t NElem>
  std::array<val_t, NElem> readNAt(std::array<size_t, NElem> &idxs) {
    std::array<val_t, NElem> result;
    readNAt(idxs, result);
    return result;
  }

};

template<size_t N>
thread_local typename TransactionalNumArray<N>::gencas_exec_t TransactionalNumArray<N>::casExecutor_ {};


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


template<typename TArray>
void readerThread(TArray& arrayRef, uint64_t seed, size_t numOps) {
  std::mt19937 engine {seed};
  std::uniform_int_distribution<uint64_t> dist {
    0, TArray::maxSize() - 1
  };
  static const size_t kReadSize = 2;
  std::array<size_t, kReadSize> idxs;
  std::array<uint64_t, kReadSize> values;
  std::set<size_t> seen;
  for (size_t i = 0; i < numOps; i++) {
    for (;;) {
      size_t idx1 = dist(engine), idx2 = dist(engine);
      if (idx1 != idx2) {
        idxs[0] = idx1;
        idxs[1] = idx2;
        break;
      }
    }
    arrayRef.readNAt(idxs, values);
    break;
  }
}

template<typename TArray>
void writerThread(TArray& arrayRef, uint64_t seed, size_t numOps) {
  std::mt19937 engine {seed};
  std::uniform_int_distribution<uint64_t> dist {
    0, TArray::maxSize() - 1
  };
  std::array<std::pair<size_t, uint64_t>, 2> toWrite;
  for (size_t i = 0; i < numOps; i++) {
    size_t idx1 {0}, idx2{0};
    for (;;) {
      idx1 = dist(engine);
      idx2 = dist(engine);
      if (idx1 != idx2) {
        break;
      }
    }
    toWrite[0].first = idx1;
    toWrite[0].second = dist(engine);
    toWrite[1].first = idx2;
    toWrite[1].second = dist(engine);
    arrayRef.writeNAt(toWrite);
  }
}


struct BenchParams {
  size_t nReaderThreads {8};
  size_t nWriterThreads {2};
  size_t getTotalThreads() const {
    return nReaderThreads + nWriterThreads;
  }
  size_t nOpsPerThread {20000};
  size_t rootSeed {0};
};

template<typename TArray>
void runOnce(TArray &numArray, const BenchParams& params) {
  vector<uint64_t> threadSeeds;
  {
    std::mt19937 topEngine {params.rootSeed};
    std::uniform_int_distribution<uint64_t> dist {
      0, std::numeric_limits<uint64_t>::max()
    };
    for (size_t i = 0; i < params.getTotalThreads();i++) {
      threadSeeds.push_back(dist(topEngine));
    }
  }
  vector<unique_ptr<thread>> threads;
  for (size_t i = 0; i < params.nReaderThreads; i++) {
    uint64_t currentSeed = threadSeeds[i];
    threads.push_back(unique_ptr<thread>{new thread{[currentSeed, &numArray, params]() {
      readerThread(numArray, currentSeed, params.nOpsPerThread);
    }}});
  }
  for (size_t i = 0; i < params.nWriterThreads; i++) {
    size_t threadIdx = i + params.nReaderThreads;
    uint64_t currentSeed = threadSeeds[threadIdx];
    threads.push_back(unique_ptr<thread>{new thread{[currentSeed, &numArray, params]() {
      writerThread(numArray, currentSeed, params.nOpsPerThread);

    }}});
  }
  for (auto& t: threads) {
    t->join();
  }
}

template<size_t NAtoms>
void runTransactional(const BenchParams& params) {
  TransactionalNumArray<NAtoms> numArray;
  runOnce(numArray, params);}


template<size_t NAtoms, typename TLockType>
void runLocking(TLockType& lockRef, const BenchParams& params) {
  LockedNumArray<NAtoms, TLockType> numArray {lockRef};
  runOnce(numArray, params);
}

void runBattery() {
  static const size_t kNAtoms = 2048;
  size_t rootSeed = 0;
  {
    std::mt19937 rootEngine {std::random_device()()};
    std::uniform_int_distribution<uint64_t> dist {
      0, std::numeric_limits<uint64_t>::max()
    };
    rootSeed = dist(rootEngine);
    // rootSeed = 1000;
  }
  BenchParams benchParams;
  benchParams.nOpsPerThread = 5000000;
  benchParams.nWriterThreads = 7;
  benchParams.nReaderThreads = 1;
  benchParams.rootSeed = rootSeed;
  Timer timer;

  {
    timer.start();
    runTransactional<kNAtoms>(benchParams);
    timer.stop();
    auto elapsed = timer.elapsedNsec();
    cout << "\t" << rightPad("tsx elapsed: ", 40) << leftPad(elapsed, 16) << endl;
  }
  {
    auto slock = PThreadSpinLock::create();
    {
      timer.start();
      runLocking<kNAtoms, PThreadSpinLock>(slock, benchParams);
      timer.stop();      
    }
    auto elapsed = timer.elapsedNsec();
    cout << "\t" << rightPad("PThreadSpinLock elapsed: ", 40) << leftPad(elapsed, 16) << endl;
  }
  {
    std::mutex mut;
    {
      timer.start();
      runLocking<kNAtoms, std::mutex>(mut, benchParams);
      timer.stop();      
    }
    auto elapsed = timer.elapsedNsec();
    cout << "\t" << rightPad("Mutex elapsed: ", 40) << leftPad(elapsed, 16) << endl;
  }  
}


int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  runBattery();

  LOG(INFO) << "end";
}
