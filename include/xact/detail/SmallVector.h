#pragma once
#include "xact/detail/vendored/chobo/small_vector.hpp"

namespace xact { namespace detail {

template<typename T>
using SmallVector = xact::detail::vendored::chobo::small_vector<T, 64>;


}} // xact::detail