#pragma once

namespace xact { namespace detail {

template<typename T1, typename T2>
struct first_type {
  using type = T1;
};


}} // xact::detail
