#pragma once
#include <cstdint>

namespace xact {
class AtomicU64;
} // xact

namespace xact { namespace generalized_cas_1 {

enum class PreconditionType: uint64_t {
  ALWAYS_TRUE = 0,
  EQ = 1,
  NEQ = 2,
  LT = 3,
  GT = 4,
  LTE = 5,
  GTE = 6
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
  PreconditionCore();
  PreconditionCore& operator=(const Precondition&);
  PreconditionCore(const Precondition&);
};

class Precondition {
 protected:
  PreconditionCore core_;
  Precondition(uint64_t *target, PreconditionType ptype, uint64_t arg1);
  Precondition(uint64_t *target, PreconditionType ptype, uint64_t arg1, uint64_t arg2);
  static Precondition makeCond(AtomicU64*, PreconditionType, uint64_t arg);
 public:
  PreconditionCore& core();
  const PreconditionCore& core() const;
  Precondition();
  static Precondition alwaysTrue();  
  static Precondition eq(AtomicU64* target, uint64_t value);
  static Precondition lt(AtomicU64* target, uint64_t value);
  static Precondition lte(AtomicU64* target, uint64_t value);
  static Precondition gt(AtomicU64* target, uint64_t value);
  static Precondition gte(AtomicU64* target, uint64_t value);
  static Precondition neq(AtomicU64* target, uint64_t value);
};

static_assert(
  sizeof(PreconditionCore) == sizeof(Precondition),
  "Precondition shouldn't have any other members."
);

}} // xact::generalized_cas_1
