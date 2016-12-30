#pragma once

#include <type_traits>
#include <utility>
#include <limits>


namespace xact { namespace detail { namespace util {

template<size_t N, typename T>
T safeCastSizeT() {
  static_assert(
    std::is_integral<T>::value,
    "T must be integral."
  );
  static_assert(
    N < std::numeric_limits<T>::max(),
    "N must be within T's max value."
  );
  return (T) N;
}

}}} // xact::detail::util
