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
#include "xact/AtomicU64.h"

using namespace std;
using xact::AtomicU64;
using xact::AtomicU64Inspector;

using namespace xact::generalized_cas_1;

TEST(TestGeneralizedCAS, TestSimpleCAS1) {
  AtomicU64 atom {20};
  auto atomPtr = AtomicU64Inspector(atom).getPointer();
  array<PreconditionCore, 1> precond;
  array<OperationCore, 1> oper;
  oper[0].typeBlock.opType = OperationType::STORE;
  oper[0].target = atomPtr;
  oper[0].arg1 = 507;

  precond[0].typeBlock.conditionType = PreconditionType::EQ;
  precond[0].target = atomPtr;
  precond[0].arg1 = 20;

  GeneralizedCAS simpleCAS;
  auto& casCore = simpleCAS.core();
  casCore.preconditions = precond.data();
  casCore.nPreconditions = 1;
  casCore.operations = oper.data();
  casCore.nOperations = 1;
  EXPECT_TRUE(simpleCAS.execute());
}

TEST(TestGeneralizedCAS, TestSimpleCAS2) {
  AtomicU64 atom {20};
  atom.store(20);
  auto atomPtr = AtomicU64Inspector(atom).getPointer();
  array<PreconditionCore, 2> precond;
  array<OperationCore, 1> oper;
  oper[0].typeBlock.opType = OperationType::STORE;
  oper[0].target = atomPtr;
  oper[0].arg1 = 507;

  precond[0].typeBlock.conditionType = PreconditionType::NEQ;
  precond[0].target = atomPtr;
  precond[0].arg1 = 10;
  // precond[1].typeBlock.conditionType = PreconditionType::LT;
  // precond[1].target = atomPtr;
  // precond[1].arg1 = 30;

  GeneralizedCAS simpleCAS;
  auto& casCore = simpleCAS.core();
  casCore.preconditions = precond.data();
  casCore.nPreconditions = 1;
  casCore.operations = oper.data();
  casCore.nOperations = 1;
  EXPECT_TRUE(simpleCAS.execute());
}
