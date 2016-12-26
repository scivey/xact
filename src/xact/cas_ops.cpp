#include "xact/cas_ops.h"
#include "xact/asm/core.h"
#include <glog/logging.h>

namespace xact {

bool tripleCAS_U64(IntegralCASParams<uint64_t>& p1,
    IntegralCASParams<uint64_t>& p2,
    IntegralCASParams<uint64_t>& p3) {
  return xact_asm_3cas_u64(
    p1.target(), p1.expected(), p1.desired(),
    p2.target(), p2.expected(), p2.desired(),
    p3.target(), p3.expected(), p3.desired()
  );
}

bool doubleCAS_U64(IntegralCASParams<uint64_t>& p1,
    IntegralCASParams<uint64_t>& p2) {
  return xact_asm_dcas_u64(
    p1.target(), p1.expected(), p1.desired(),
    p2.target(), p2.expected(), p2.desired()
  );
}


bool singleCAS_U64(IntegralCASParams<uint64_t>& p1) {
  return xact_asm_scas_u64(
    p1.target(), p1.expected(), p1.desired()
  );
}

} // xact
