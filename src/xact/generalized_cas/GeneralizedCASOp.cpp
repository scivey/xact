#include "xact/generalized_cas/GeneralizedCASOp.h"
#include "xact/generalized_cas/Operation.h"
#include "xact/generalized_cas/Precondition.h"
#include "xact/detail/LockManager.h"
#include "xact/detail/asm/generalized_cas.h"
#include "xact/TransactionStatus.h"
#include "xact/LockableAtomicU64.h"
#include "xact/detail/util/ScopeGuard.h"
#include "xact/detail/SmallVector.h"

#include <glog/logging.h>
#include <set>
#include <vector>

namespace xact { namespace generalized_cas {


TransactionStatus GeneralizedCASOp::lockAndExecute() {
  using atom_t = xact_lockable_atomic_u64_t;
  using atom_vec = xact::detail::SmallVector<atom_t*>;
  atom_vec atoms;
  atoms.reserve(core_.nPreconditions + core_.nOperations);

  for (size_t i = 0; i < core_.nPreconditions; i++) {
    auto precond = core_.preconditions + i;
    auto atomPtr = precond->target;
    atoms.emplace_back((atom_t*) atomPtr);
  }
  for (size_t i = 0; i < core_.nOperations; i++) {
    auto op = core_.operations + i;
    auto atomPtr = op->target;
    atoms.emplace_back((atom_t*) atomPtr);
  }
  atom_t** dataPtr = atoms.data();

  auto lockManager = detail::LockManager::create(dataPtr, atoms.size());

  for (;;) {
    if (lockManager.tryLockForWrite()) {
      auto guard = detail::util::makeGuard([&lockManager]() {
        lockManager.unlockFromWrite();
      });
      return this->executeWithLocksHeld();
    }
  }
}

}} // xact::generalized_cas

