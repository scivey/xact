#pragma once
#include <cstdint>

namespace xact { namespace generalized_cas_1 {

enum class OperationType: uint64_t {
  NULLOP = 0,
  STORE = 1,
  FETCH_ADD = 2
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
  OperationCore& operator=(const Operation&);
};

class Operation {
 protected:
  OperationCore core_;
  Operation(uint64_t *target, OperationType opType, uint64_t arg1);
  Operation(uint64_t *target, OperationType opType, uint64_t arg1, uint64_t arg2);
 public:
  Operation();
  OperationCore& core();
  const OperationCore& core() const;
  static Operation nullOp();
  static Operation store(uint64_t *target, uint64_t value); 
  static Operation fetchAdd(uint64_t *target, uint64_t *result, uint64_t addBy);
};


}} // xact::generalized_cas_1
