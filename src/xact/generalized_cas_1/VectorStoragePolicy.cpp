#include "xact/generalized_cas_1/VectorStoragePolicy.h"
#include "xact/generalized_cas_1/Operation.h"
#include "xact/generalized_cas_1/Precondition.h"

namespace xact { namespace generalized_cas_1 {

Precondition* VectorStoragePolicy::getPreconditionStorage() {
  return preconditions_.data();
}

size_t VectorStoragePolicy::getPreconditionCount() {
  return preconditions_.size();
}

Operation* VectorStoragePolicy::getOperationStorage() {
  return operations_.data();
}

size_t VectorStoragePolicy::getOperationCount() {
  return operations_.size();
}

void VectorStoragePolicy::pushPrecondition(Precondition&& condition) {
  preconditions_.emplace_back(std::forward<Precondition>(condition));
}

void VectorStoragePolicy::pushOperation(Operation&& operation) {
  operations_.emplace_back(std::forward<Operation>(operation));
}

void VectorStoragePolicy::clearPreconditionStorage() {
  preconditions_.clear();
}

void VectorStoragePolicy::clearOperationStorage() {
  operations_.clear();
}

}} // xact::generalized_cas_1
