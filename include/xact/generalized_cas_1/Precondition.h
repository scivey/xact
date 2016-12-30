#pragma once
#include <cstdint>

namespace xact { namespace generalized_cas_1 {

enum class PreconditionType: uint64_t {
  ALWAYS_TRUE = 0,
  EQ = 1,
  NEQ = 2,
  LT = 3,
  GT = 4,
  LTE = 5
};

struct PreconditionTypeBlock {
  PreconditionType conditionType {PreconditionType::ALWAYS_TRUE};
};


class Precondition;

struct PreconditionCore {
  uint64_t *target {nullptr};
  PreconditionTypeBlock typeBlock;
  uint64_t arg1 {0};
  uint64_t arg2 {0};
  PreconditionCore& operator=(const Precondition&);
};

class Precondition {
 protected:
  PreconditionCore core_;
  Precondition(uint64_t *target, PreconditionType ptype, uint64_t arg1);
  Precondition(uint64_t *target, PreconditionType ptype, uint64_t arg1, uint64_t arg2);
 public:
  PreconditionCore& core();
  const PreconditionCore& core() const;
  Precondition();
  static Precondition alwaysTrue();
  static Precondition equals(uint64_t *target, uint64_t value);
  static Precondition lessThan(uint64_t *target, uint64_t value);
  static Precondition greaterThan(uint64_t *target, uint64_t value);
  static Precondition notEquals(uint64_t *target, uint64_t value);
};


}} // xact::generalized_cas_1
