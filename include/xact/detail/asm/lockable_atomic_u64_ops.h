#pragma once
#include <stdint.h>
#include "xact/detail/asm/lockable_atomic_u64.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64_t xact_lockable_atomic_u64_init(volatile xact_lockable_atomic_u64_t *target, uint64_t value);

uint64_t xact_lockable_atomic_u64_load(volatile xact_lockable_atomic_u64_t *target,
    volatile uint64_t *result);

uint64_t xact_lockable_atomic_u64_store(volatile xact_lockable_atomic_u64_t *target, uint64_t value);

uint64_t xact_lockable_atomic_u64_compare_exchange(
    volatile xact_lockable_atomic_u64_t *target,
    volatile uint64_t *expected, uint64_t desired);

uint64_t xact_lockable_atomic_u64_fetch_add(
    volatile xact_lockable_atomic_u64_t *target,
    volatile uint64_t *result, uint64_t addBy);


uint64_t xact_lockable_atomic_u64_incr(volatile xact_lockable_atomic_u64_t*);

uint64_t xact_lockable_atomic_u64_is_locked(volatile xact_lockable_atomic_u64_t*);

uint64_t xact_lockable_atomic_u64_trylock(volatile xact_lockable_atomic_u64_t*);

uint64_t xact_lockable_atomic_u64_lock(volatile xact_lockable_atomic_u64_t*);

uint64_t xact_lockable_atomic_u64_unlock(volatile xact_lockable_atomic_u64_t*);

#ifdef __cplusplus
}
#endif
