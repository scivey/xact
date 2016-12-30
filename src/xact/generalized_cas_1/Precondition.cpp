#include "xact/generalized_cas_1/Precondition.h"
#include "xact/AtomicU64.h"

namespace xact { namespace generalized_cas_1 {


PreconditionCore::PreconditionCore(){}

PreconditionCore::PreconditionCore(const Precondition& precond) {
  *this = precond;
}

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

Precondition Precondition::makeCond(AtomicU64 *target, PreconditionType ptype,
    uint64_t value) {
  return Precondition {
    AtomicU64Inspector(*target).getPointer(),
    ptype,
    value    
  };
}

Precondition Precondition::eq(AtomicU64 *target, uint64_t value) {
  return makeCond(target, PreconditionType::EQ, value);
}

Precondition Precondition::lt(AtomicU64 *target, uint64_t value) {
  return makeCond(target, PreconditionType::LT, value);
}

Precondition Precondition::lte(AtomicU64 *target, uint64_t value) {
  return makeCond(target, PreconditionType::LTE, value);
}

Precondition Precondition::gt(AtomicU64 *target, uint64_t value) {
  return makeCond(target, PreconditionType::GT, value);
}

Precondition Precondition::gte(AtomicU64 *target, uint64_t value) {
  return makeCond(target, PreconditionType::GTE, value);
}  

Precondition Precondition::neq(AtomicU64 *target, uint64_t value) {
  return makeCond(target, PreconditionType::NEQ, value);
}

}} // xact::generalized_cas_1
