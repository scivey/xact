#include "xact/generalized_cas_1/Operation.h"
#include <cstdint>

namespace xact { namespace generalized_cas_1 {

OperationCore& OperationCore::operator=(const Operation& oper) {
  *this = oper.core();
  return *this;
} 

Operation::Operation(){}

Operation::Operation(uint64_t *target, OperationType opType, uint64_t arg1) {
  core_.target = target;
  core_.typeBlock.opType = opType;
  core_.arg1 = arg1;
}

Operation::Operation(uint64_t *target, OperationType opType, uint64_t arg1, uint64_t arg2) {
  core_.target = target;
  core_.typeBlock.opType = opType;
  core_.arg1 = arg1;
  core_.arg2 = arg2;  
}

OperationCore& Operation::core() {
  return core_;
}

const OperationCore& Operation::core() const {
  return core_;
}

Operation Operation::nullOp() {
  return Operation{};
}

Operation Operation::store(uint64_t *target, uint64_t value) {
  return Operation{target, OperationType::STORE, value};
}

Operation Operation::fetchAdd(uint64_t *target, uint64_t *result, uint64_t addBy) {
  return Operation{target, OperationType::FETCH_ADD, (uint64_t) result, addBy};
}

}} // xact::generalized_cas_1
