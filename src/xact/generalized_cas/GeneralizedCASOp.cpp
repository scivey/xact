#include "xact/generalized_cas/GeneralizedCASOp.h"
#include "xact/generalized_cas/Operation.h"
#include "xact/generalized_cas/Precondition.h"
#include "xact/detail/asm/generalized_cas.h"
#include "xact/TransactionStatus.h"
#include "xact/LockableAtomicU64.h"
#include <glog/logging.h>
#include <set>
#include <vector>

namespace xact { namespace generalized_cas {

GeneralizedCASOpCore& GeneralizedCASOp::core() {
  return core_;
}

const GeneralizedCASOpCore& GeneralizedCASOp::core() const {
  return core_;
}

TransactionStatus GeneralizedCASOp::executeInternal() {
  const uint64_t nLimitedRetries = 8;
  return transactionStatusFromRax(xact_generalized_cas_op_tsx_with_retries(
    core_.preconditions, core_.nPreconditions,
    core_.operations, core_.nOperations,
    nLimitedRetries
  ));
}

TransactionStatus GeneralizedCASOp::tryLockAndExecute() {
  std::set<uintptr_t> distinctAtoms;
  for (size_t i = 0; i < core_.nPreconditions; i++) {
    auto precond = core_.preconditions + i;
    auto atomPtr = precond->target;
    auto uptr = (uintptr_t) atomPtr;
    if (distinctAtoms.count(uptr) == 0) {
      distinctAtoms.insert(uptr);
    }
  }
  for (size_t i = 0; i < core_.nOperations; i++) {
    auto op = core_.operations + i;
    auto atomPtr = op->target;
    auto uptr = (uintptr_t) atomPtr;
    if (distinctAtoms.count(uptr) == 0) {
      distinctAtoms.insert(uptr);
    }
  }
  std::vector<LockableAtomicU64*> lockedAtoms;
  lockedAtoms.reserve(distinctAtoms.size());
  size_t lockedIdx = 0;
  for (auto uPtr: distinctAtoms) {
    auto asAtom = (LockableAtomicU64*) uPtr;
    if (asAtom->tryLock()) {
      lockedIdx++;
      lockedAtoms.push_back(asAtom);
    }
  }
  if (lockedAtoms.size() != distinctAtoms.size()) {
    while (!lockedAtoms.empty()) {
      lockedAtoms.back()->unlock();
      lockedAtoms.pop_back();
    }
    return TransactionStatus::RESOURCE_LOCKED;
  }
  auto retCode = xact_generalized_cas_op_with_locks_acquired(
    core_.preconditions, core_.nPreconditions,
    core_.operations, core_.nOperations
  );
  auto result = transactionStatusFromAbortCode(retCode);
  while (!lockedAtoms.empty()) {
    lockedAtoms.back()->unlock();
    lockedAtoms.pop_back();
  }
  return result;
}



TransactionStatus GeneralizedCASOp::lockAndExecute() {
  for (;;) {
    auto status = tryLockAndExecuteFixedSize<32>();
    if (status != TransactionStatus::RESOURCE_LOCKED) {
      return status;
    }
  }
}

}} // xact::generalized_cas

