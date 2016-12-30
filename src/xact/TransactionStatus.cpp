#include "xact/TransactionStatus.h"


namespace xact {

TransactionStatus transactionStatusFromAbortCode(uint64_t rc) {
  switch(rc) {
    case 0:
      return TransactionStatus::OK;
    case 1:
      return TransactionStatus::RESOURCE_LOCKED;
    case 2:
      return TransactionStatus::PRECONDITION_FAILED;
    case 3:
      return TransactionStatus::INVALID_PRECONDITION;
    case 4:
      return TransactionStatus::INVALID_OPERATION;
    default:
      return TransactionStatus::TSX_UNKNOWN_FAILURE;
  }
}

TransactionStatus transactionStatusFromInt(uint64_t rc) {
  switch(rc) {
    case 0:
      return TransactionStatus::OK;
    case 1:
      return TransactionStatus::RESOURCE_LOCKED;
    case 2:
      return TransactionStatus::PRECONDITION_FAILED;
    case 3:
      return TransactionStatus::INVALID_PRECONDITION;
    case 4:
      return TransactionStatus::INVALID_OPERATION;
    case 101:
      return TransactionStatus::TSX_RETRY;
    case 102:
      return TransactionStatus::TSX_CONFLICT;
    case 103:
      return TransactionStatus::TSX_CAPACITY_EXCEEDED;
    case 198:
      return TransactionStatus::TSX_ZERO_FAILURE;
    case 199:
      return TransactionStatus::TSX_UNKNOWN_FAILURE;
    default:
      return TransactionStatus::TSX_UNKNOWN_FAILURE;
  }
}

static const uint64_t kTSXExplicit = uint64_t(1) << 0;
static const uint64_t kTSXRetry = uint64_t(1) << 1;
static const uint64_t kTSXConflict = uint64_t(1) << 2;
static const uint64_t kTSXCapacity = uint64_t(1) << 3;

TransactionStatus transactionStatusFromRax(uint64_t rc) {
  {
    // TSX sometimes calls the abort handler with an error code.
    // To handle this, our asm error handlers check for rax==0
    // and, if detected, set the return value to 0x00b33f00.
    // (see generalized_cas.asm)
    uint32_t beefCheck = *((uint32_t*) &rc);
    if (beefCheck == 0x00b33f00) {
      return TransactionStatus::TSX_ZERO_FAILURE;
    }
  }

  // unless we hit `beefCheck` above, the TSX
  // abort type should be in the first byte of `rc`.
  // if it was an explicit abort (triggered by `xabort imm8`),
  // the 4th byte will contain `imm8`.

  uint8_t *u8ptr = (uint8_t*) &rc;
  uint8_t tsxCode = *u8ptr;
  uint8_t explicitCode = *(u8ptr+3);
  uint64_t ts64 = tsxCode;
  uint64_t exp64 = explicitCode;
  if (ts64 == 0) {
    return TransactionStatus::OK;
  }
  if (ts64 & kTSXExplicit) {
    return transactionStatusFromAbortCode(exp64);
  }
  if (ts64 & kTSXRetry) {
    return TransactionStatus::TSX_RETRY;
  }
  if (ts64 & kTSXConflict) {
    return TransactionStatus::TSX_CONFLICT;
  }
  if (ts64 & kTSXCapacity) {
    return TransactionStatus::TSX_CAPACITY_EXCEEDED;
  }
  return TransactionStatus::TSX_UNKNOWN_FAILURE;
}

} // xact
