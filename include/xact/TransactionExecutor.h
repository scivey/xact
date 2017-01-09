#pragma once
#include "xact/detail/macros.h"
#include "xact/detail/asm/util.h"
#include "xact/TransactionStatus.h"
#include <random>

namespace xact {

class DefaultTransactionRetryPolicy {
 public:
  template<typename TTransaction>
  bool shouldRetry(TTransaction& xRef, TransactionStatus status, size_t nRetries) {
    if (nRetries > 1000) {
      return false;
    }
    switch (status) {
      case TransactionStatus::TSX_RETRY:
      case TransactionStatus::TSX_ZERO_FAILURE:
      case TransactionStatus::TSX_CONFLICT:
      case TransactionStatus::RESOURCE_LOCKED:
        return true;
      case TransactionStatus::PRECONDITION_FAILED:
      case TransactionStatus::INVALID_PRECONDITION:
      case TransactionStatus::INVALID_OPERATION:
      case TransactionStatus::EMPTY:
        return false;
      default:
        if (nRetries < 20) {
          return true;
        }
        return false;
    }
  }
  template<typename TTransaction>
  TransactionStatus onFailedTransaction(TTransaction& xRef, TransactionStatus status,
      size_t nRetries) {
    switch (status) {
      case TransactionStatus::TSX_CONFLICT:
      case TransactionStatus::TSX_ZERO_FAILURE:
      case TransactionStatus::TSX_RETRY:
      case TransactionStatus::TSX_CAPACITY_EXCEEDED:
      case TransactionStatus::TSX_UNKNOWN_FAILURE:
      case TransactionStatus::RESOURCE_LOCKED:
        return xRef.lockAndExecute();

      case TransactionStatus::PRECONDITION_FAILED:
      case TransactionStatus::INVALID_OPERATION:
      case TransactionStatus::INVALID_PRECONDITION:
      case TransactionStatus::EMPTY:
        return status;
      default:
        return xRef.lockAndExecute();
    }
  }
};


template<typename TRetryPolicy = DefaultTransactionRetryPolicy>
class TransactionExecutor: public TRetryPolicy {
 protected:
  std::mt19937 randomEngine_ {std::random_device()()};
  std::uniform_int_distribution<uint64_t> dist_ {1, 100};
 public:
  template<typename TTransaction>
  TransactionStatus execute(TTransaction& transaction) {
    auto result = transaction.execute();
    size_t nTries = 1;    
    while (result != TransactionStatus::OK && this->shouldRetry(transaction, result, nTries)) {
      xact_busy_wait(dist_(randomEngine_));
      result = transaction.execute();
      nTries++;
    }
    if (result != TransactionStatus::OK) {
      result = this->onFailedTransaction(transaction, result, nTries);
    }
    XACT_RW_BARRIER();
    return result;
  }
  template<typename TTransaction>
  TransactionStatus execute(TTransaction&& transaction) {
    return execute(transaction);
  }
};

} // xact


