#include "xact/atomic_ops/single.h"
#include "xact_asm/core.h"

namespace xact { namespace atomic_ops {


bool casU64S(uint64_t *atomTarget, uint64_t expected, uint64_t desired,
    uint64_t *actual) {
  uint64_t tempExpected = expected;
  auto rc = xact_atomic_cas_u64_single(atomTarget, &tempExpected, desired);
  if (rc == 0) {
    return true;
  }
  if (actual != nullptr) {
    *actual = tempExpected;
  }
  return false;
}

void storeU64S(uint64_t *atomTarget, uint64_t desired) {
  xact_atomic_store_u64_single(atomTarget, desired);
}

uint64_t loadU64S(uint64_t *atomTarget) {
  return xact_atomic_load_u64_single(atomTarget);
}

uint64_t fetchAddU64S(uint64_t *atomTarget, uint64_t addBy) {
  return xact_atomic_fetch_add_u64_single(atomTarget, addBy);
}

uint64_t fetchSubU64S(uint64_t *atomTarget, uint64_t subBy) {
  return xact_atomic_fetch_sub_u64_single(atomTarget, subBy);
}

}} // xact::atomic_ops
