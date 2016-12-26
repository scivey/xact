#pragma once
#include <cstdint>
#include "xact/IntegralCASParams.h"

namespace xact {

// a single CAS doesn't really need a transaction - this is just for testing
bool singleCAS_U64(IntegralCASParams<uint64_t>& target1);

bool doubleCAS_U64(IntegralCASParams<uint64_t>& target1, IntegralCASParams<uint64_t>& target2);

bool tripleCAS_U64(IntegralCASParams<uint64_t>& target1,
    IntegralCASParams<uint64_t>& target2,
    IntegralCASParams<uint64_t>& target3);

} // xact
