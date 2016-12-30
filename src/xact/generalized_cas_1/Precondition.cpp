#include "xact/generalized_cas_1/Precondition.h"

namespace xact { namespace generalized_cas_1 {

PreconditionCore& PreconditionCore::operator=(const Precondition& precond) {
  *this = precond.core();
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

Precondition Precondition::equals(uint64_t *target, uint64_t value) {
  return Precondition{target, PreconditionType::EQ, value};
}

Precondition Precondition::lessThan(uint64_t *target, uint64_t value) {
  return Precondition{target, PreconditionType::LT, value};
}

Precondition Precondition::greaterThan(uint64_t *target, uint64_t value) {
  return Precondition{target, PreconditionType::GT, value};
}  

Precondition Precondition::notEquals(uint64_t *target, uint64_t value) {
  return Precondition{target, PreconditionType::NEQ, value};
}

}} // xact::generalized_cas_1
