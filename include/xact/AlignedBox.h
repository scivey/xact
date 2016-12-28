#pragma once
#include <type_traits>
#include <utility>
#include <cstddef>

namespace xact {

template<typename T, size_t NAlignment = 64>
struct AlignedBox {
  T value;

  struct DefaultConstructionTag {};

  template<typename U = T>
  AlignedBox(const typename std::enable_if<std::is_default_constructible<U>::value, DefaultConstructionTag>::type& param = DefaultConstructionTag()){}

  template<typename ...Types>
  AlignedBox(Types&&... args): value(std::forward<Types>(args)...) {}

  AlignedBox& operator=(T val) {
    value = val;
    return *this;
  }

  T* ptr() {
    return &value;
  }

  AlignedBox& operator+=(T val) {
    value += val;
    return *this;
  }

  operator T() {
    return value;
  }

} __attribute__((aligned(NAlignment)));

} // xact
