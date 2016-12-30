#pragma once
#include <cstdint>

namespace xact { namespace generalized_cas {

enum class OperationType: uint64_t {
  NOOP = 0,
  STORE = 1,
  FETCH_ADD = 2
};

struct OperationTypeBlock {
  OperationType opType {OperationType::NOOP};
  // uint8_t padding[7];
};

struct OperationCore {
  uint64_t *target {nullptr};
  OperationTypeBlock typeBlock;
  uint64_t arg1 {0};
  uint64_t arg2 {0};
};


}} // xact::generalized_cas
