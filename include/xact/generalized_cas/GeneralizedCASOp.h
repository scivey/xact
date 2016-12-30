#pragma once
#include <cstddef>
#include <array>
#include <algorithm>

#include "xact/generalized_cas/Operation.h"
#include "xact/generalized_cas/Precondition.h"
#include "xact/TransactionStatus.h"
#include "xact/detail/macros.h"
#include "xact/detail/asm/generalized_cas.h"
#include "xact/LockableAtomicU64.h"


namespace xact { namespace generalized_cas {

struct GeneralizedCASOpCore {
  PreconditionCore *preconditions {nullptr};
  size_t nPreconditions {0};
  OperationCore *operations {nullptr};
  size_t nOperations {0};
};

class GeneralizedCASOp {
 protected:
  GeneralizedCASOpCore core_;
  TransactionStatus executeInternal();
 public:
  GeneralizedCASOpCore& core();
  const GeneralizedCASOpCore& core() const;

  inline TransactionStatus execute() {
    auto result = executeInternal();
    return result;
  }

  TransactionStatus lockAndExecute();
  TransactionStatus tryLockAndExecute();  

  template<size_t N>
  TransactionStatus tryLockAndExecuteFixedSize() {
    std::array<uintptr_t, N> atoms;
    size_t arrayIdx;
    for (size_t i = 0; i < core_.nPreconditions; i++) {
      auto precond = core_.preconditions + i;
      atoms[arrayIdx] = (uintptr_t) precond->target;
      arrayIdx++;
    }
    for (size_t i = 0; i < core_.nOperations; i++) {
      auto op = core_.operations + i;
      atoms[arrayIdx] = (uintptr_t) op->target;
      arrayIdx++;
    }
    std::sort(atoms.begin(), atoms.begin()+arrayIdx);
    uintptr_t prev = 0;
    std::array<uintptr_t, N> distinctAtoms;
    size_t nDistinctAtoms = 0;
    for (size_t i = 0; i < arrayIdx; i++) {
      auto current = atoms[i];
      if (prev != 0 && current == prev) {
        continue;
      }
      distinctAtoms[nDistinctAtoms] = current;
      nDistinctAtoms++;
    }
    size_t lockedIdx = 0;
    for (size_t i = 0; i < nDistinctAtoms; i++) {
      auto asAtom = (LockableAtomicU64*) distinctAtoms[i];
      if (asAtom->tryLock()) {
        lockedIdx++;
      } else {
        break;
      }
    }
    if (lockedIdx != nDistinctAtoms) {
      for (size_t i = lockedIdx; i > 0; i--) {
        auto asAtom = (LockableAtomicU64*) distinctAtoms[i-1];
        asAtom->unlock();
      }
    }
    auto retCode = xact_generalized_cas_op_with_locks_acquired(
      core_.preconditions, core_.nPreconditions,
      core_.operations, core_.nOperations
    );
    auto result = transactionStatusFromAbortCode(retCode);
    for (size_t i = lockedIdx; i > 0; i--) {
      auto asAtom = (LockableAtomicU64*) distinctAtoms[i-1];
      asAtom->unlock();
    }
    return result;
  }
};

}} // xact::generalized_cas
