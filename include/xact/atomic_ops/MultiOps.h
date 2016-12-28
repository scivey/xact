#pragma once
#include "xact/atomic_ops/multi.h"

namespace xact { namespace atomic_ops {

template<typename TFlatPolicy>
class UniformNonCASMultiRetryPolicy: public TFlatPolicy {
 public:
  static size_t addRetries() {
    return TFlatPolicy::numRetries();
  }
  static size_t subRetries() {
    return TFlatPolicy::numRetries();
  }  
  static size_t loadRetries() {
    return TFlatPolicy::numRetries();
  }
  static size_t storeRetries() {
    return TFlatPolicy::numRetries();
  }
};

template<typename TFlatPolicy>
class FlatCASRetryPolicy: public TFlatPolicy {
 public:
  static size_t casRetries() {
    return TFlatPolicy::numRetries();
  }
};

template<size_t N>
class FixedNumRetries {
 public:
  static size_t numRetries() {
    return N;
  }
};

template<typename TSimplePolicy, typename TCASPolicy = TSimplePolicy>
class UniformMultiRetryPolicy: public UniformNonCASMultiRetryPolicy<TSimplePolicy>,
                               public FlatCASRetryPolicy<TCASPolicy> {};


template<typename TRetryPolicy = UniformMultiRetryPolicy<FixedNumRetries<0>>>
class MultiOps: public TRetryPolicy {
 public:
  template<typename ...Types>
  bool addU64M(Types&&... args) {
    auto maxRetries = this->addRetries();
    size_t nAttempts = 0;
    do {
      if (atomic_ops::addU64M(std::forward<Types>(args)...)) {
        return true;
      }
      nAttempts++;
    } while (nAttempts < maxRetries);
    return false;
  }  
  template<typename ...Types>
  bool fetchAddU64M(Types&&... args) {
    auto maxRetries = this->addRetries();
    size_t nAttempts = 0;
    do {
      if (atomic_ops::fetchAddU64M(std::forward<Types>(args)...)) {
        return true;
      }
      nAttempts++;
    } while (nAttempts < maxRetries);
    return false;
  }
  template<typename ...Types>
  bool subU64M(Types&&... args) {
    auto maxRetries = this->subRetries();
    size_t nAttempts = 0;
    do {
      if (atomic_ops::subU64M(std::forward<Types>(args)...)) {
        return true;
      }
      nAttempts++;
    } while (nAttempts < maxRetries);
    return false;
  }
  template<typename ...Types>
  bool fetchSubU64M(Types&&... args) {
    auto maxRetries = this->subRetries();
    size_t nAttempts = 0;
    do {
      if (atomic_ops::fetchSubU64M(std::forward<Types>(args)...)) {
        return true;
      }
      nAttempts++;
    } while (nAttempts < maxRetries);
    return false;
  }  
  template<typename ...Types>
  bool casU64M(Types&&... args) {
    auto maxRetries = this->casRetries();
    size_t nAttempts = 0;
    do {
      if (atomic_ops::casU64M(std::forward<Types>(args)...)) {
        return true;
      }
      nAttempts++;
    } while (nAttempts < maxRetries);
    return false;
  }
  template<typename ...Types>
  bool storeU64M(Types&&... args) {
    auto maxRetries = this->storeRetries();
    size_t nAttempts = 0;
    do {
      if (atomic_ops::storeU64M(std::forward<Types>(args)...)) {
        return true;
      }
      nAttempts++;
    } while (nAttempts < maxRetries);
    return false;
  }
  template<typename ...Types>
  bool loadU64M(Types&&... args) {
    auto maxRetries = this->loadRetries();
    size_t nAttempts = 0;
    do {
      if (atomic_ops::loadU64M(std::forward<Types>(args)...)) {
        return true;
      }
      nAttempts++;
    } while (nAttempts < maxRetries);
    return false;
  }
};


}} // xact::atomic_ops