#pragma once
#include <cstddef>
#include "xact/generalized_cas/Operation.h"
#include "xact/generalized_cas/Precondition.h"

namespace xact { namespace generalized_cas {

struct GeneralizedCASCore {
  PreconditionCore *preconditions {nullptr};
  size_t nPreconditions {0};
  OperationCore *operations {nullptr};
  size_t nOperations {0};
};

class GeneralizedCAS {
 protected:
  GeneralizedCASCore core_;
 public:
  GeneralizedCASCore& core();
  const GeneralizedCASCore& core() const;
  bool execute();
};

}} // xact::generalized_cas