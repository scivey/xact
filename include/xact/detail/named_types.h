#pragma once

#include "xact/detail/NamedType.h"

namespace xact { namespace detail {

using UseTSX = detail::NamedType<bool, struct UseTSXTag>;

}} // xact::detail
