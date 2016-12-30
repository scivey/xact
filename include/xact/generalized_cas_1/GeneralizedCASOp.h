#pragma once
#include <cstddef>
#include "xact/generalized_cas_1/Operation.h"
#include "xact/generalized_cas_1/Precondition.h"

namespace xact { namespace generalized_cas_1 {

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
  GeneralizedCASOpCore& core();
  const GeneralizedCASOpCore& core() const;
  bool execute();
};

}} // xact::generalized_cas_1
