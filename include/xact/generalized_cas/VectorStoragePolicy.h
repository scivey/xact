#pragma once

#include "xact/generalized_cas/Operation.h"
#include "xact/generalized_cas/Precondition.h"
#include "xact/detail/SmallVector.h"
#include <cstddef>
#include <vector>

namespace xact { namespace generalized_cas {

class VectorStoragePolicy {
 protected:
  detail::SmallVector<Precondition> preconditions_;
  detail::SmallVector<Operation> operations_;
 public:
  Precondition* getPreconditionStorage();
  size_t getPreconditionCount() const;
  Operation* getOperationStorage();
  size_t getOperationCount() const;
  void pushPrecondition(const Precondition& condition);
  void pushPrecondition(Precondition&& condition);
  void pushOperation(const Operation& operation);
  void pushOperation(Operation&& operation);

  void clearPreconditionStorage();
  void clearOperationStorage();
};

}} // xact::generalized_cas
