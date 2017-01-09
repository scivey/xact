#pragma once
#include <type_traits>
#include <sstream>
#include <iostream>
#include "xact/detail/traits.h"

namespace xact { namespace detail {

template<typename T, typename TUniqueTag>
class NamedType {
 protected:
  T value_;
 public:
  using underlying_type = T;
  using unique_tag_type = TUniqueTag;

  explicit NamedType(T&& val): value_(std::forward<T>(val)) {}

  operator T() const {
    return value_;
  }

  struct DefaultConstructionParam {};

  template<typename U>
  using conditional_default = typename std::enable_if<
    std::is_default_constructible<U>::value,
    DefaultConstructionParam
  >::type;

  template<typename U = T>
  NamedType(
    const typename conditional_default<U>::type& parm = DefaultConstructionParam()
  ){}

  T& value() {
    return value_;
  }
  const T& value() const {
    return value_;
  }

  template<typename U = T>
  typename first_type<bool, decltype(std::declval<T>() != std::declval<T>())>::type
  operator!=(const NamedType& other) const {
    return value_ != other.value_;
  }
  template<typename U = T>
  typename first_type<bool, decltype(std::declval<T>() < std::declval<T>())>::type
  operator<(const NamedType& other) const {
    return value_ < other.value_;
  }

  template<typename U = T>
  typename first_type<std::ostream&, decltype(std::declval<std::ostream>() << std::declval<T>())>::type
  operator<<(std::ostream& oss) const {
    oss << value();
    return oss;
  }
};


}} // xact::detail
