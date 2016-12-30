#pragma once
#include "xact/generalized_cas/GeneralizedCASOp.h"
#include "xact/generalized_cas/Precondition.h"
#include "xact/generalized_cas/Operation.h"
#include "xact/generalized_cas/VectorStoragePolicy.h"
#include "xact/generalized_cas/ExceptionErrorPolicy.h"
#include "xact/detail/macros.h"
#include "xact/detail/asm/util.h"
#include "xact/TransactionStatus.h"
#include <random>

namespace xact { namespace generalized_cas {


class DefaultCASExecutorRetryPolicy {
 public:
  template<typename TCasOp>
  bool shouldRetry(TCasOp& opRef, TransactionStatus status, size_t nRetries) {
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
  template<typename TCasOp>  
  TransactionStatus onFailedTransaction(TCasOp& casOp, TransactionStatus status, size_t nRetries) {
    switch (status) {
      case TransactionStatus::TSX_CONFLICT:
      case TransactionStatus::TSX_ZERO_FAILURE:
      case TransactionStatus::TSX_RETRY:
      case TransactionStatus::TSX_CAPACITY_EXCEEDED:
      case TransactionStatus::TSX_UNKNOWN_FAILURE:
      case TransactionStatus::RESOURCE_LOCKED:
        return casOp.lockAndExecute();

      case TransactionStatus::PRECONDITION_FAILED:
      case TransactionStatus::INVALID_OPERATION:
      case TransactionStatus::INVALID_PRECONDITION:
      case TransactionStatus::EMPTY:
        return status;
      default:
        return casOp.lockAndExecute();
    }
  } 
};

template<typename TRetryPolicy = DefaultCASExecutorRetryPolicy>
class GeneralizedCASExecutor: public TRetryPolicy {
 protected:
  std::mt19937 randomEngine_ {std::random_device()()};
  std::uniform_int_distribution<uint64_t> dist_ {1, 100};
 public:
  template<typename TCasOp>
  TransactionStatus execute(TCasOp& casOp) {
    auto result = casOp.execute();
    size_t nTries = 1;
    if (result != TransactionStatus::OK) {
      while (result != TransactionStatus::OK && this->shouldRetry(casOp, result, nTries)) {
        xact_busy_wait(dist_(randomEngine_));
        result = casOp.execute();
        nTries++;
      }
    }

    if (result != TransactionStatus::OK) {
      result = this->onFailedTransaction(casOp, result, nTries);
    }
    return result;
  }
};


}} // xact::generalized_cas

