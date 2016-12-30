#include "xact/generalized_cas/VectorStoragePolicy.h"
#include "xact/generalized_cas/Operation.h"
#include "xact/generalized_cas/Precondition.h"

namespace xact { namespace generalized_cas {

Precondition* VectorStoragePolicy::getPreconditionStorage() {
  return preconditions_.data();
}

size_t VectorStoragePolicy::getPreconditionCount() const {
  return preconditions_.size();
}

Operation* VectorStoragePolicy::getOperationStorage() {
  return operations_.data();
}

size_t VectorStoragePolicy::getOperationCount() const {
  return operations_.size();
}

void VectorStoragePolicy::pushPrecondition(Precondition&& condition) {
  preconditions_.emplace_back(std::forward<Precondition>(condition));
}

void VectorStoragePolicy::pushPrecondition(const Precondition& condition) {
  preconditions_.emplace_back(condition);
}

void VectorStoragePolicy::pushOperation(Operation&& operation) {
  operations_.emplace_back(std::forward<Operation>(operation));
}

void VectorStoragePolicy::pushOperation(const Operation& operation) {
  operations_.push_back(operation);
}

void VectorStoragePolicy::clearPreconditionStorage() {
  preconditions_.clear();
}

void VectorStoragePolicy::clearOperationStorage() {
  operations_.clear();
}

}} // xact::generalized_cas
