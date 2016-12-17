#include "xact/cas_ops.h"
#include "xact/asm/core.h"
#include <glog/logging.h>

namespace xact {


bool doubleCAS_U64(IntegralCASParams<uint64_t>& p1,
    IntegralCASParams<uint64_t>& p2) {
  LOG(INFO) << *p1.target() << "\t" << *p1.expected() << "\t" << p1.desired();
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
