#pragma once
#include <stdint.h>
#include "xact/detail/asm/lockable_atomic_u64.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64_t xact_lockable_atomic_u64_init(xact_lockable_atomic_u64_t *target, uint64_t value);

uint64_t xact_lockable_atomic_u64_load_tsx(xact_lockable_atomic_u64_t *target,
    uint64_t *result);

uint64_t xact_lockable_atomic_u64_store_tsx(xact_lockable_atomic_u64_t *target, uint64_t value);

uint64_t xact_lockable_atomic_u64_compare_exchange(
    xact_lockable_atomic_u64_t *target,
    uint64_t *expected, uint64_t desired);

uint64_t xact_lockable_atomic_u64_fetch_add_tsx(
    xact_lockable_atomic_u64_t *target,
    uint64_t *result, uint64_t addBy);


uint64_t xact_lockable_atomic_u64_incr(xact_lockable_atomic_u64_t*);
uint64_t xact_lockable_atomic_u64_load_raw_value_dangerously(xact_lockable_atomic_u64_t*);
uint64_t xact_lockable_atomic_u64_compare_exchange_cmpxchg16b(xact_lockable_atomic_u64_t*,
    uint64_t *expected, uint64_t desired);
uint64_t xact_lockable_atomic_u64_load_mvcc(xact_lockable_atomic_u64_t *target, uint64_t *result);

uint64_t xact_lockable_atomic_u64_store_cmpxchg16b(xact_lockable_atomic_u64_t* target, uint64_t value);

uint64_t xact_lockable_atomic_u64_fetch_add_cmpxchg16b(xact_lockable_atomic_u64_t* target,
    uint64_t *result, uint64_t add_by);
#ifdef __cplusplus
}
#endif
