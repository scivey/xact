#pragma once
#include <cstdint>

namespace xact {
class LockableAtomicU64;
} // xact

namespace xact { namespace generalized_cas {

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
  PreconditionCore& operator=(const PreconditionCore&);
  PreconditionCore(const Precondition&);
  PreconditionCore(const PreconditionCore&);
};

class Precondition {
 protected:
  PreconditionCore core_;
  Precondition(uint64_t *target, PreconditionType ptype, uint64_t arg1);
  Precondition(uint64_t *target, PreconditionType ptype, uint64_t arg1, uint64_t arg2);
  static Precondition makeCond(LockableAtomicU64*, PreconditionType, uint64_t arg);
 public:
  Precondition();
  Precondition(const Precondition&);
  Precondition& operator=(const Precondition&);
  bool isEmpty() const;
  PreconditionCore& core();
  const PreconditionCore& core() const;
  static Precondition alwaysTrue();
  static Precondition eq(LockableAtomicU64* target, uint64_t value);
  static Precondition lt(LockableAtomicU64* target, uint64_t value);
  static Precondition lte(LockableAtomicU64* target, uint64_t value);
  static Precondition gt(LockableAtomicU64* target, uint64_t value);
  static Precondition gte(LockableAtomicU64* target, uint64_t value);
  static Precondition neq(LockableAtomicU64* target, uint64_t value);
};

static_assert(
  sizeof(PreconditionCore) == sizeof(Precondition),
  "Precondition shouldn't have any other members."
);

}} // xact::generalized_cas
