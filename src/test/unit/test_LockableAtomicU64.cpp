#include <gtest/gtest.h>
#include <string>
#include <functional>
#include <thread>
#include <chrono>

#include <atomic>
#include <memory>
#include <vector>
#include <atomic>
#include <glog/logging.h>

#include "xact/generalized_cas/GeneralizedCAS.h"
#include "xact/generalized_cas/Operation.h"
#include "xact/generalized_cas/Precondition.h"
#include "xact/detail/asm/util.h"
#include "xact/detail/asm/lockable_atomic_u64_ops.h"
#include "xact/detail/macros.h"

#include "xact/LockableAtomicU64.h"
#include "xact/TransactionStatus.h"

using namespace std;
using xact::LockableAtomicU64;
using xact::LockableAtomicU64Inspector;
using xact::TransactionStatus;

using namespace xact::generalized_cas;

uint64_t* valPtr(LockableAtomicU64 *target) {
  return LockableAtomicU64Inspector(*target).getPointer();
}



TEST(TestLockableAtomicU64, TestSingleThreadedCAS2) {
  LockableAtomicU64 atom {0};
  atom.store(0);
  uint64_t value = 0;
  CHECK(atom.store(value) == TransactionStatus::OK);
  size_t nSuccess {0};
  while (nSuccess < 50000) {    
    for (;;) {
      uint64_t expected = 0;
      TransactionStatus res = TransactionStatus::OK;
      for (;;) {
        res = atom.load(&expected);
        if (res == TransactionStatus::TSX_RETRY || res == TransactionStatus::TSX_CONFLICT) {
          continue;
        }
        break;
      }
      uint64_t desired = expected + 1;
      res = atom.compareExchange(&expected, desired);
      if (res == TransactionStatus::OK) {
        nSuccess++;
        break;
      }
    }
  }
  uint64_t result;
  EXPECT_EQ(atom.load(&result), TransactionStatus::OK);
  EXPECT_EQ(50000, result);
}




TEST(TestLockableAtomicU64, TestMultithreadedStupidCAS1) {
  LockableAtomicU64 atom {0};
  static const size_t kIncrPerThread = 50000;
  static const size_t kNThreads = 4;
  XACT_MFENCE_BARRIER();
  vector<unique_ptr<thread>> threads;
  for (size_t i = 0; i < kNThreads; i++) {
    threads.push_back(unique_ptr<thread>{new thread{[&atom]() {
      size_t nSuccess = 0;
      while (nSuccess < kIncrPerThread) {
        uint64_t expected = 0;
        for (;;) {
          {
            auto res = atom.load(&expected);
            if (res != TransactionStatus::OK) {
              continue;
            }
          }
          XACT_MFENCE_BARRIER();
          uint64_t desired = expected + 1;
          XACT_MFENCE_BARRIER();          
          auto res = atom.compareExchange(&expected, desired);
          if (res == TransactionStatus::OK) {
            nSuccess++;
            break;
          }
        }
      }
      LOG(INFO) << "nSuccess: " << nSuccess;
    }}});
  }
  XACT_MFENCE_BARRIER();
  for (auto& t: threads) {
    t->join();
  }
  XACT_MFENCE_BARRIER();  
  uint64_t result {0};
  XACT_MFENCE_BARRIER();  
  EXPECT_EQ(atom.load(&result), TransactionStatus::OK);
  XACT_MFENCE_BARRIER();  
  EXPECT_EQ(kNThreads*kIncrPerThread, result);
}

