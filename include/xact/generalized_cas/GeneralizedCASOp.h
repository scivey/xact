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
 public:
  inline GeneralizedCASOpCore& core() {
    return core_;
  }
  inline const GeneralizedCASOpCore& core() const {
    return core_;
  }

  inline TransactionStatus execute() {
    const uint64_t kLimitedRetries = 8;
    auto retCode = xact_generalized_cas_op_tsx_with_retries(
      core_.preconditions, core_.nPreconditions,
      core_.operations, core_.nOperations,
      kLimitedRetries
    );
    return transactionStatusFromRax(retCode);
  }

  inline TransactionStatus executeWithLocksHeld() {
    auto retCode = xact_generalized_cas_op_with_locks_acquired(
      core_.preconditions, core_.nPreconditions,
      core_.operations, core_.nOperations
    );
    return transactionStatusFromAbortCode(retCode);
  }

  TransactionStatus lockAndExecute();
};

}} // xact::generalized_cas
