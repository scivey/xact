#include "xact/generalized_cas/Precondition.h"
#include "xact/LockableAtomicU64.h"

namespace xact { namespace generalized_cas {


PreconditionCore::PreconditionCore(){}

PreconditionCore::PreconditionCore(const Precondition& precond) {
  *this = precond;
}

PreconditionCore& PreconditionCore::operator=(const Precondition& precond) {
  *this = precond.core();
  return *this;
}

PreconditionCore& PreconditionCore::operator=(const PreconditionCore& other) {
  target = other.target;
  typeBlock.conditionType = other.typeBlock.conditionType;
  arg1 = other.arg1;
  arg2 = other.arg2;
  return *this;
}

PreconditionCore::PreconditionCore(const PreconditionCore& other) {
  *this = other;
}

Precondition::Precondition(const Precondition& other): core_(other.core()) {}

Precondition& Precondition::operator=(const Precondition& other) {
  core_ = other.core_;
  return *this;
}

Precondition::Precondition(uint64_t *target, PreconditionType ptype, uint64_t arg1) {
  core_.target = target;
  core_.typeBlock.conditionType = ptype;
  core_.arg1 = arg1;
}

Precondition::Precondition(uint64_t *target, PreconditionType ptype, uint64_t arg1, uint64_t arg2) {
  core_.target = target;
  core_.typeBlock.conditionType = ptype;
  core_.arg1 = arg1;
  core_.arg2 = arg2;
}

Precondition::Precondition(){}

PreconditionCore& Precondition::core() {
  return core_;
}

const PreconditionCore& Precondition::core() const {
  return core_;
}

Precondition Precondition::alwaysTrue() {
  return Precondition{};
}

Precondition Precondition::makeCond(LockableAtomicU64 *target, PreconditionType ptype,
    uint64_t value) {
  return Precondition {
    LockableAtomicU64Inspector(*target).getPointer(),
    ptype,
    value    
  };
}

Precondition Precondition::eq(LockableAtomicU64 *target, uint64_t value) {
  return makeCond(target, PreconditionType::EQ, value);
}

Precondition Precondition::lt(LockableAtomicU64 *target, uint64_t value) {
  return makeCond(target, PreconditionType::LT, value);
}

Precondition Precondition::lte(LockableAtomicU64 *target, uint64_t value) {
  return makeCond(target, PreconditionType::LTE, value);
}

Precondition Precondition::gt(LockableAtomicU64 *target, uint64_t value) {
  return makeCond(target, PreconditionType::GT, value);
}

Precondition Precondition::gte(LockableAtomicU64 *target, uint64_t value) {
  return makeCond(target, PreconditionType::GTE, value);
}  

Precondition Precondition::neq(LockableAtomicU64 *target, uint64_t value) {
  return makeCond(target, PreconditionType::NEQ, value);
}

bool Precondition::isEmpty() const {
  return !!core_.target;
}

}} // xact::generalized_cas
