#include <gtest/gtest.h>
#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <vector>
#include <atomic>

#include "xact/generalized_cas_1/GeneralizedCAS.h"
#include "xact/generalized_cas_1/Operation.h"
#include "xact/generalized_cas_1/Precondition.h"
#include "xact/generalized_cas_1/GeneralizedCAS.h"
#include "xact/AtomicU64.h"

using namespace std;
using xact::AtomicU64;
using xact::AtomicU64Inspector;

using namespace xact::generalized_cas_1;

uint64_t* getPointer(AtomicU64& atom) {
  return AtomicU64Inspector(atom).getPointer();
}

using gencas_t = GeneralizedCAS<VectorStoragePolicy, ExceptionErrorPolicy>;

TEST(TestGeneralizedCAS, TestSanity1) {
  AtomicU64 atom {20};
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
  AtomicU64 atom {20};
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
  AtomicU64 atom {20};
  gencas_t genc;
  genc.push(Precondition::eq(&atom, 20));
  genc.push(Operation::store(&atom, 507));
  EXPECT_TRUE(genc.execute());
  EXPECT_EQ(507, atom.load());
}

TEST(TestGeneralizedCAS, TestSimpleCAS2) {
  AtomicU64 atom {20};

  gencas_t genc;
  genc.push(Precondition::lt(&atom, 30));
  genc.push(Precondition::gt(&atom, 10));
  genc.push(Operation::store(&atom, 919));

  EXPECT_TRUE(genc.execute());
  EXPECT_EQ(919, atom.load());
}


TEST(TestGeneralizedCAS, TestCompare2FetchAdd2Others) {
  std::array<AtomicU64, 4> atoms {10, 20, 30, 40};
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
  EXPECT_TRUE(genc.execute());
  EXPECT_EQ(100, atoms[2].load());
  EXPECT_EQ(200, atoms[3].load());
}


TEST(TestGeneralizedCAS, TestDoubleCASMultiThreaded) {
  std::array<AtomicU64, 2> atoms {0, 0};
  static const size_t kIncrPerThread = 50000;
  vector<unique_ptr<thread>> threads;
  for (size_t i = 0; i < 4; i++) {
    threads.push_back(unique_ptr<thread>{new thread{[&atoms]() {
      uint64_t expected[2];
      uint64_t desired[2];
      for (size_t i = 0; i < kIncrPerThread; i++) {
        for (;;) {
          expected[0] = atoms[0].load();
          expected[1] = atoms[1].load();
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
          if (genc.execute()) {
            break;
          }
        }
      }
    }}});
  }
  for (auto& t: threads) {
    t->join();
  }
  EXPECT_EQ(4*kIncrPerThread, atoms[0].load());
  EXPECT_EQ(4*kIncrPerThread, atoms[1].load());
}


