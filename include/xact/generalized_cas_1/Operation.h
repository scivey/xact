#pragma once
#include <cstdint>

namespace xact {
class AtomicU64;
}


namespace xact { namespace generalized_cas_1 {

enum class OperationType: uint64_t {
  NULLOP = 0,
  STORE = 1,
  FETCH_ADD = 2,
  LOAD = 3
};

struct OperationTypeBlock {
  OperationType opType {OperationType::NULLOP};
};

class Operation;

struct OperationCore {
  uint64_t *target {nullptr};
  OperationTypeBlock typeBlock;
  uint64_t arg1 {0};
  uint64_t arg2 {0};
  OperationCore();
  OperationCore& operator=(const Operation&);
  OperationCore(const Operation&);
};

class Operation {
 protected:
  OperationCore core_;
  Operation(uint64_t *target, OperationType opType, uint64_t arg1);
  Operation(uint64_t *target, OperationType opType, uint64_t arg1, uint64_t arg2);
  static Operation makeOp(AtomicU64*, OperationType, uint64_t);
  static Operation makeOp(AtomicU64*, OperationType, uint64_t, uint64_t);
 public:
  Operation();
  OperationCore& core();
  const OperationCore& core() const;
  static Operation nullOp();
  static Operation store(AtomicU64 *target, uint64_t value);
  static Operation load(AtomicU64 *target, uint64_t* dest);
  static Operation fetchAdd(AtomicU64 *target, uint64_t *result, uint64_t addBy);
};

static_assert(
  sizeof(OperationCore) == sizeof(Operation),
  "Operation shouldn't have any other members."
);
}} // xact::generalized_cas_1
