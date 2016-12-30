#pragma once

namespace xact { namespace generalized_cas {


enum class PreconditionType: uint64_t {
  ALWAYS_TRUE = 0,
  EQ = 1,
  NEQ = 2,
  LT = 3,
  GT = 4
};

struct PreconditionTypeBlock {
  PreconditionType conditionType {PreconditionType::ALWAYS_TRUE};
};

struct PreconditionCore {
  uint64_t *target {nullptr};
  PreconditionTypeBlock typeBlock;
  uint64_t arg1 {0};
  uint64_t arg2 {0};
};

// class Precondition {
//  protected:
//   PreconditionCore core_;
//  public:
//   Precondition
// };

}} // xact::generalized_cas
