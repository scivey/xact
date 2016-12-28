#include "xact/atomic_ops/multi.h"
#include "xact_asm/core.h"

namespace xact { namespace atomic_ops {

bool casU64M(uint64_t **atomTargets, const uint64_t *expecteds,
    const uint64_t *desireds, uint8_t n) {
  return xact_atomic_cas_u64_multi(
    atomTargets, expecteds, desireds, n
  ) == 0;
}

bool storeU64M(uint64_t **atomTargets, const uint64_t *sources, uint8_t n) {
  return xact_atomic_store_u64_multi(
    atomTargets, sources, n
  ) == 0;
}

bool loadU64M(uint64_t **atomTargets, uint64_t *dests, uint8_t n) {
  return xact_atomic_load_u64_multi(
    atomTargets, dests, n
  ) == 0;  
}

bool addU64M(uint64_t **atomTargets, uint64_t amount, uint8_t n) {
  return xact_atomic_add_u64_multi(
    atomTargets, amount, n
  ) == 0;
}

bool subU64M(uint64_t **atomTargets, uint64_t amount, uint8_t n) {
  return xact_atomic_sub_u64_multi(
    atomTargets, amount, n
  ) == 0;
}

bool fetchAddU64M(uint64_t **atomTargets, uint64_t *results, uint64_t amount, uint8_t n) {
  return xact_atomic_fetch_add_u64_multi(
    atomTargets, results, amount, n
  ) == 0;
}

bool fetchSubU64M(uint64_t **atomTargets, uint64_t *results, uint64_t amount, uint8_t n) {
  return xact_atomic_fetch_sub_u64_multi(
    atomTargets, results, amount, n
  ) == 0;
}

}} // xact::atomic_ops
