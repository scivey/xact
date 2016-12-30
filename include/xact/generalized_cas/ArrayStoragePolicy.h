#pragma once

#include <array>
#include <cstddef>
#include <vector>
#include "xact/generalized_cas/Operation.h"
#include "xact/generalized_cas/Precondition.h"

namespace xact { namespace generalized_cas {

template<size_t N>
class ArrayStoragePolicy {
 protected:
  std::array<Precondition, N> preconditions_;
  std::array<Operation, N> operations_;
  size_t nPreconditions_ {0};
  size_t nOperations_ {0};
 public:
  Precondition* getPreconditionStorage() {
    return preconditions_.data();
  }
  size_t getPreconditionCount() const {
    return nPreconditions_;
  }
  Operation* getOperationStorage() {
    return operations_.data();
  }
  size_t getOperationCount() const {
    return nOperations_;
  }
  void pushPrecondition(const Precondition& condition) {
    preconditions_[nPreconditions_] = condition;
    nPreconditions_++;
    DCHECK(nPreconditions_ < N);
  }
  void pushPrecondition(Precondition&& condition) {
    preconditions_[nPreconditions_] = std::move(condition);
    nPreconditions_++;
    DCHECK(nPreconditions_ < N);
  }
  void pushOperation(const Operation& operation) {
    operations_[nOperations_] = operation;
    nOperations_++;
    DCHECK(nOperations_ < N);
  }
  void pushOperation(Operation&& operation) {
    operations_[nOperations_] = std::move(operation);
    nOperations_++;
    DCHECK(nOperations_ < N);    
  }
  void clearPreconditionStorage() {
    nPreconditions_ = 0;
  }
  void clearOperationStorage() {
    nOperations_ = 0;
  }
};

}} // xact::generalized_cas
