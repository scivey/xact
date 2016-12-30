#pragma once
#include "xact/generalized_cas_1/GeneralizedCASOp.h"
#include "xact/generalized_cas_1/Precondition.h"
#include "xact/generalized_cas_1/Operation.h"
#include "xact/generalized_cas_1/VectorStoragePolicy.h"
#include "xact/generalized_cas_1/ExceptionErrorPolicy.h"

namespace xact { namespace generalized_cas_1 {


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
  bool execute() {
    if (!hasOperations()) {
      return this->onEmptyExecution();
    }
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

    return casOp.execute();
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


}} // xact::generalized_cas_1