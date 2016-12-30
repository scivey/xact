#include "xact/generalized_cas/Operation.h"
#include "xact/LockableAtomicU64.h"
#include <cstdint>


namespace xact { namespace generalized_cas {

OperationCore::OperationCore(const Operation& oper) {
  *this = oper;
}

OperationCore& OperationCore::operator=(const Operation& oper) {
  *this = oper.core();
  return *this;
} 

OperationCore::OperationCore(){}
OperationCore& OperationCore::operator=(const OperationCore& other){
  target = other.target;
  typeBlock.opType = other.typeBlock.opType;
  arg1 = other.arg1;
  arg2 = other.arg2;
  return *this;
}

OperationCore::OperationCore(const OperationCore& other) {
  *this = other;
}

Operation::Operation(){}
Operation::Operation(const Operation& other): core_(other.core_) {}
Operation& Operation::operator=(const Operation& other) {
  core_ = other.core_;
  return *this;
}

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


Operation Operation::makeOp(LockableAtomicU64 *target, OperationType opType, uint64_t value) {
  return Operation{
    LockableAtomicU64Inspector(*target).getPointer(),
    opType,
    value
  };
}

Operation Operation::makeOp(LockableAtomicU64 *target, OperationType opType,
    uint64_t arg1, uint64_t arg2) {
  return Operation{
    LockableAtomicU64Inspector(*target).getPointer(),
    opType,
    arg1,
    arg2
  };
}


Operation Operation::nullOp() {
  return Operation{};
}

Operation Operation::store(LockableAtomicU64 *target, uint64_t value) {
  return makeOp(target, OperationType::STORE, value);
}

Operation Operation::fetchAdd(LockableAtomicU64 *target, uint64_t *result, uint64_t addBy) {
  return makeOp(target, OperationType::FETCH_ADD, (uint64_t) result, addBy);
}

Operation Operation::load(LockableAtomicU64 *target, uint64_t *result) {
  return makeOp(target, OperationType::LOAD, (uint64_t) result);
}

bool Operation::isEmpty() const {
  return !!core_.target;
}

}} // xact::generalized_cas_1
