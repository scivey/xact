#pragma once
#include <utility>
#include <type_traits>

namespace xact {

template<typename T>
class IntegralCASParams {
 public:
  using value_type = T;
  static_assert(std::is_integral<T>::value, "T must be an integral type.");
 protected:
  T *target_ {nullptr};
  T *expected_ {nullptr};
  T desired_;
 public:
  T* target() {
    return target_;
  }
  const T* target() const {
    return target_;
  }
  IntegralCASParams& target(T *targetPtr) {
    target_ = targetPtr;
    return *this;
  }
  T* expected() {
    return expected_;
  }
  const T* expected() const {
    return expected_;
  }
  IntegralCASParams& expected(T *expectedPtr) {
    expected_ = expectedPtr;
    return *this;
  }
  T desired() const {
    return desired_;
  }
  IntegralCASParams& desired(T desiredVal) {
    desired_ = desiredVal;
    return *this;
  }
};

} // xact