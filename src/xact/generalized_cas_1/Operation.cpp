#include "xact/generalized_cas_1/Operation.h"
#include "xact/AtomicU64.h"
#include <cstdint>


namespace xact { namespace generalized_cas_1 {

OperationCore::OperationCore(const Operation& oper) {
  *this = oper;
}

OperationCore& OperationCore::operator=(const Operation& oper) {
  *this = oper.core();
  return *this;
} 

OperationCore::OperationCore(){}

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

Operation Operation::makeOp(AtomicU64 *target, OperationType opType, uint64_t value) {
  return Operation{
    AtomicU64Inspector(*target).getPointer(),
    opType,
    value
  };
}

Operation Operation::makeOp(AtomicU64 *target, OperationType opType,
    uint64_t arg1, uint64_t arg2) {
  return Operation{
    AtomicU64Inspector(*target).getPointer(),
    opType,
    arg1,
    arg2
  };
}


Operation Operation::nullOp() {
  return Operation{};
}

Operation Operation::store(AtomicU64 *target, uint64_t value) {
  return makeOp(target, OperationType::STORE, value);
}

Operation Operation::fetchAdd(AtomicU64 *target, uint64_t *result, uint64_t addBy) {
  return makeOp(target, OperationType::FETCH_ADD, (uint64_t) result, addBy);
}

Operation Operation::load(AtomicU64 *target, uint64_t *result) {
  return makeOp(target, OperationType::LOAD, (uint64_t) result);
}

}} // xact::generalized_cas_1
