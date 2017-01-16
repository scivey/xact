#include <gtest/gtest.h>
#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <vector>
#include <atomic>

#include "xact/generalized_cas/GeneralizedCAS.h"
#include "xact/TransactionExecutor.h"
#include "xact/generalized_cas/Operation.h"
#include "xact/generalized_cas/Precondition.h"
#include "xact/detail/asm/util.h"
#include "xact/detail/macros.h"

#include "xact/LockableAtomicU64.h"
#include "xact/TransactionStatus.h"
#include "xact_testing/ThreadGroup.h"
using namespace std;
using xact::LockableAtomicU64;
using xact::detail::LockableAtomicU64Inspector;
using xact::TransactionStatus;
using xact_testing::ThreadGroup;
using namespace xact;
using namespace xact::generalized_cas;

uint64_t* getPointer(LockableAtomicU64& atom) {
  return LockableAtomicU64Inspector(atom).getPointer();
}

using gencas_t = GeneralizedCAS<VectorStoragePolicy, ExceptionErrorPolicy>;
using gencas_exec_t = TransactionExecutor<DefaultTransactionRetryPolicy>;


TEST(TestGeneralizedCAS, TestSanity1) {
  LockableAtomicU64 atom {20};
  gencas_t genc;
  EXPECT_TRUE(genc.empty());
  genc.push(Precondition::lt(&atom, 50));
  EXPECT_FALSE(genc.empty());
  genc.clear();
  EXPECT_TRUE(genc.empty());
  genc.push(Operation::store(&atom, 100));
  EXPECT_FALSE(genc.empty());
}

TEST(TestGeneralizedCAS, TestEmptyExecution) {
  bool thrown {false};
  gencas_t genc;
  try {
    genc.execute();
  } catch (const std::runtime_error& ex) {
    thrown = true;
  }
  EXPECT_TRUE(thrown);
  thrown = false;
  LockableAtomicU64 atom {20};
  genc.clear();
  genc.push(Operation::store(&atom, 100));
  try {
    genc.execute();
  } catch (const std::runtime_error& ex) {
    thrown = true;
  }
  EXPECT_FALSE(thrown);
}


TEST(TestGeneralizedCAS, TestSimpleCAS1) {
  LockableAtomicU64 atom {20};
  gencas_t genc;
  gencas_exec_t executor;
  genc.push(Precondition::eq(&atom, 20));
  genc.push(Operation::store(&atom, 507));
  EXPECT_EQ(executor.execute(genc), TransactionStatus::OK);
  uint64_t result {0};
  EXPECT_EQ(atom.load(&result), TransactionStatus::OK);
  EXPECT_EQ(507, result);
}

TEST(TestGeneralizedCAS, TestSimpleCAS2) {
  LockableAtomicU64 atom {20};
  gencas_t genc;
  gencas_exec_t executor;  
  genc.push(Precondition::lt(&atom, 30));
  genc.push(Precondition::gt(&atom, 10));
  genc.push(Operation::store(&atom, 919));

  EXPECT_EQ(executor.execute(genc), TransactionStatus::OK);
  uint64_t result;
  EXPECT_EQ(atom.load(&result), TransactionStatus::OK);
  EXPECT_EQ(919, result);
}


TEST(TestGeneralizedCAS, TestCompare2FetchAdd2OthersHappy) {
  std::array<LockableAtomicU64, 4> atoms {10, 20, 30, 40};
  gencas_exec_t executor;
  gencas_t genc {
    {
      Precondition::gt(&atoms[0], 5),
      Precondition::lt(&atoms[0], 15),
      Precondition::eq(&atoms[1], 20),
      Precondition::eq(&atoms[2], 30),
      Precondition::eq(&atoms[3], 40)
    },
    {
      Operation::store(&atoms[2], 100),
      Operation::store(&atoms[3], 200)
    }
  };
  EXPECT_EQ(executor.execute(genc), TransactionStatus::OK);
  uint64_t result {0};
  EXPECT_EQ(atoms[2].load(&result), TransactionStatus::OK);
  EXPECT_EQ(100, result);
  EXPECT_EQ(atoms[3].load(&result), TransactionStatus::OK);
  EXPECT_EQ(200, result);
}

TEST(TestGeneralizedCAS, TestCompare2FetchAdd2OthersSad) {
  std::array<LockableAtomicU64, 4> atoms {10, 20, 30, 40};
  gencas_t genc {
    {
      Precondition::gt(&atoms[0], 5),
      Precondition::lt(&atoms[0], 8),
      Precondition::eq(&atoms[1], 20),
      Precondition::eq(&atoms[2], 30),
      Precondition::eq(&atoms[3], 40)
    },
    {
      Operation::store(&atoms[2], 100),
      Operation::store(&atoms[3], 200)
    }
  };
  gencas_exec_t executor;
  EXPECT_EQ(executor.execute(genc), TransactionStatus::PRECONDITION_FAILED);
  uint64_t result {0};
  for (size_t i = 0; i < 4; i++) {
    size_t expected = (i+1) * 10;
    EXPECT_EQ(atoms[i].load(&result), TransactionStatus::OK);
    EXPECT_EQ(expected, result);
  }
}


TEST(TestGeneralizedCAS, TestDCASHappy) {
  array<LockableAtomicU64, 2> atoms {10, 20};
  gencas_t genc {
    {
      Precondition::eq(&atoms[0], 10),
      Precondition::eq(&atoms[1], 20)
    },
    {
      Operation::store(&atoms[0], 100),
      Operation::store(&atoms[1], 200)
    }
  };
  gencas_exec_t executor;
  EXPECT_EQ(executor.execute(genc), TransactionStatus::OK);
}

TEST(TestGeneralizedCAS, TestDCASSad1) {
  array<LockableAtomicU64, 2> atoms {10, 20};
  gencas_t genc {
    {
      Precondition::eq(&atoms[0], 11),
      Precondition::eq(&atoms[1], 20)
    },
    {
      Operation::store(&atoms[0], 100),
      Operation::store(&atoms[1], 200)
    }
  };
  gencas_exec_t executor;
  EXPECT_EQ(executor.execute(genc), TransactionStatus::PRECONDITION_FAILED);
}

TEST(TestGeneralizedCAS, TestDCASSad2) {
  array<LockableAtomicU64, 2> atoms {10, 20};
  gencas_t genc {
    {
      Precondition::eq(&atoms[0], 10),
      Precondition::eq(&atoms[1], 21)
    },
    {
      Operation::store(&atoms[0], 100),
      Operation::store(&atoms[1], 200)
    }
  };
  gencas_exec_t executor;  
  EXPECT_EQ(executor.execute(genc), TransactionStatus::PRECONDITION_FAILED);
}


TEST(TestGeneralizedCAS, TestDoubleCASSingleThreaded) {
  array<LockableAtomicU64, 2> atoms {0, 0};
  static const size_t kIncrPerThread = 10000;
  static const size_t kNThreads = 1;
  static const uint64_t kExpectedVal = kNThreads * kIncrPerThread;
  {
    uint64_t expected[2] = {0, 0};
    uint64_t desired[2] = {0, 0};
    size_t nSuccess = 0;
    gencas_exec_t executor;
    while (nSuccess < kIncrPerThread) {
      for (;;) {
        {
          gencas_t loadTransaction {
            {},
            {
              Operation::load(&atoms[0], &expected[0]),
              Operation::load(&atoms[1], &expected[1]),
            }
          };
          executor.execute(loadTransaction);
        }
        desired[0] = expected[0] + 1;
        desired[1] = expected[1] + 1;
        gencas_t genc {
          {
            Precondition::eq(&atoms[0], expected[0]),
            Precondition::eq(&atoms[1], expected[1])
          },
          {
            Operation::store(&atoms[0], desired[0]),
            Operation::store(&atoms[1], desired[1])
          }
        };
        auto outcome = executor.execute(genc);
        if (outcome == TransactionStatus::OK) {
          nSuccess++;
          break;
        }
      }
    }
  }
  uint64_t result {0};
  EXPECT_EQ(atoms[0].load(&result), TransactionStatus::OK);
  EXPECT_EQ(kExpectedVal, result);
  EXPECT_EQ(atoms[1].load(&result), TransactionStatus::OK);
  EXPECT_EQ(kExpectedVal, result);
}


TEST(TestGeneralizedCAS, TestDoubleCASMultiThreaded) {
  static const size_t kIncrPerThread = 10000;
  static const size_t kNThreads = 2;
  static const size_t kExpectedVal = kNThreads * kIncrPerThread;
  std::array<LockableAtomicU64, 2> atoms {0, 0};
  XACT_MFENCE_BARRIER();
  auto threads = ThreadGroup::createShared(kNThreads, [&atoms](size_t) {
    uint64_t expected[2] = {0, 0};
    gencas_exec_t executor;
    uint64_t desired[2] = {0, 0};
    size_t nSuccess = 0;
    while (nSuccess < kIncrPerThread) {
      for (;;) {
        {
          gencas_t loadTransaction {
            {},
            {
              Operation::load(&atoms[0], &expected[0]),
              Operation::load(&atoms[1], &expected[1]),
            }
          };
          auto res = executor.execute(loadTransaction);
          EXPECT_EQ(TransactionStatus::OK, res);
        }
        desired[0] = expected[0] + 1;
        desired[1] = expected[1] + 1;
        gencas_t genc {
          {
            Precondition::eq(&atoms[0], expected[0]),
            Precondition::eq(&atoms[1], expected[1])
          },
          {
            Operation::store(&atoms[0], desired[0]),
            Operation::store(&atoms[1], desired[1])
          }
        };
        auto outcome = executor.execute(genc);
        if (outcome == TransactionStatus::OK) {
          nSuccess++;
          break;
        }
      }
    }
  });
  threads->join();
  uint64_t result {0};
  EXPECT_EQ(atoms[0].load(&result), TransactionStatus::OK);
  EXPECT_EQ(kExpectedVal, result);
  EXPECT_EQ(atoms[1].load(&result), TransactionStatus::OK);
  EXPECT_EQ(kExpectedVal, result);
}


