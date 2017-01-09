#pragma once
#include "xact/generalized_cas/GeneralizedCASOp.h"
#include "xact/generalized_cas/Precondition.h"
#include "xact/generalized_cas/Operation.h"
#include "xact/generalized_cas/VectorStoragePolicy.h"
#include "xact/generalized_cas/ExceptionErrorPolicy.h"
#include "xact/detail/macros.h"

#include "xact/TransactionStatus.h"

namespace xact { namespace generalized_cas {


template<
  typename TStoragePolicy = VectorStoragePolicy,
  typename TErrorPolicy = ExceptionErrorPolicy
>
class GeneralizedCAS: public TStoragePolicy,
                      public TErrorPolicy {
 public:
  using error_policy_t = TErrorPolicy;
  using storage_policy_t = TStoragePolicy;
  using cond_init_list = std::initializer_list<Precondition>;
  using op_init_list = std::initializer_list<Operation>;

  GeneralizedCAS(){}

  template<typename TCondOrOpSeq>
  GeneralizedCAS(TCondOrOpSeq&& elems) {
    for (auto&& elem: elems) {
      push(std::forward<decltype(elem)>(elem));
    }
  }

  GeneralizedCAS(cond_init_list&& elems) {
    for (auto&& elem: elems) {
      push(std::forward<decltype(elem)>(elem));
    }
  }

  GeneralizedCAS(cond_init_list&& conds, op_init_list&& ops) {
    for (auto&& cond: conds) {
      push(std::forward<decltype(cond)>(cond));
    }
    for (auto&& op: ops) {
      push(std::forward<decltype(op)>(op));
    }    
  }

  template<typename TCondOrOpSeq1, typename TCondOrOpSeq2>
  GeneralizedCAS(TCondOrOpSeq1&& elems1, TCondOrOpSeq2&& elems2) {
    for (auto&& elem: elems1) {
      push(std::forward<decltype(elem)>(elem));
    }
    for (auto&& elem: elems2) {
      push(std::forward<decltype(elem)>(elem));
    }
  }  

  bool hasOperations() const {
    return this->getOperationCount() > 0;
  }
  bool hasPreconditions() const {
    return this->getPreconditionCount() > 0;
  }
  bool empty() const {
    return !hasOperations() && !hasPreconditions();
  }
 protected:
  GeneralizedCASOp buildCASOp() {
    auto casOp = GeneralizedCASOp();
    auto& opCore = casOp.core();
    Precondition* preconditions = this->getPreconditionStorage();
    static_assert(
      sizeof(Precondition) == sizeof(PreconditionCore),
      "Precondition size assumptions violated"
    );
    opCore.preconditions = (PreconditionCore*) preconditions;
    opCore.nPreconditions = this->getPreconditionCount();

    Operation *operations = this->getOperationStorage();
    static_assert(
      sizeof(Operation) == sizeof(OperationCore),
      "Operation size assumptions violated"
    );
    opCore.operations = (OperationCore*) operations;
    opCore.nOperations = this->getOperationCount();
    return casOp;
  }
 public:
  inline TransactionStatus execute() {
    auto casOp = buildCASOp();
    if (casOp.core().nOperations == 0) {
      this->onEmptyExecution();
      return TransactionStatus::EMPTY;      
    }
    auto result = casOp.execute();
    return result;
  }
  TransactionStatus lockAndExecute() {
    auto casOp = buildCASOp();
    if (casOp.core().nOperations == 0) {
      this->onEmptyExecution();
      return TransactionStatus::EMPTY;
    }
    return casOp.lockAndExecute();
  }
  void push(Precondition&& condition) {
    this->pushPrecondition(std::forward<Precondition>(condition));
  }
  void push(const Precondition& cond) {
    this->pushPrecondition(cond);
  }
  void push(Operation&& operation) {
    this->pushOperation(std::forward<Operation>(operation));
  }
  void push(const Operation& op) {
    this->pushOperation(op);
  }
  void clear() {
    this->clearPreconditionStorage();
    this->clearOperationStorage();
  }
};


}} // xact::generalized_cas

