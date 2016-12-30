#pragma once
#include <cstdint>

namespace xact {

enum class TransactionStatus: uint64_t {
  OK = 0,
  RESOURCE_LOCKED = 1,
  PRECONDITION_FAILED = 2,
  INVALID_PRECONDITION = 3,
  INVALID_OPERATION = 4,

  TSX_RETRY = 101,
  TSX_CONFLICT = 102,
  TSX_CAPACITY_EXCEEDED = 103,
  TSX_ZERO_FAILURE = 198,
  TSX_UNKNOWN_FAILURE = 199,

  EMPTY = 201
};

TransactionStatus transactionStatusFromInt(uint64_t);
TransactionStatus transactionStatusFromRax(uint64_t);
TransactionStatus transactionStatusFromAbortCode(uint64_t);

} // xact
