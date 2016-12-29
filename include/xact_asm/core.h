#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool xact_eq_test(uint64_t x, uint64_t* y);
uint64_t xact_add_test(uint64_t* x, uint64_t* y);

void xact_mfence();

void xact_atomic_store_u64_single(uint64_t *target, uint64_t value);
uint64_t xact_atomic_load_u64_single(uint64_t *target);
uint64_t xact_atomic_fetch_add_u64_single(uint64_t *target, uint64_t value);
int xact_atomic_conditional_fetch_add_u64_single_if_between(uint64_t *target, uint64_t *result,
    uint64_t addBy, uint64_t lower_inclusive, uint64_t upper_exclusive);

uint64_t xact_atomic_fetch_sub_u64_single(uint64_t *target, uint64_t value);

int xact_atomic_cas_u64_single(uint64_t *target,
    uint64_t *expected, uint64_t desired);

int xact_atomic_store_u64_multi(uint64_t **atomics, const uint64_t *sources, uint8_t n);
int xact_atomic_load_u64_multi(uint64_t **atomics, uint64_t *dests, uint8_t n);
int xact_atomic_add_u64_multi(uint64_t **atomics, uint64_t amount, uint8_t n);
int xact_atomic_fetch_add_u64_multi(uint64_t **atomics, uint64_t *results, uint64_t amount, uint8_t n);

int xact_atomic_sub_u64_multi(uint64_t **atomics, uint64_t amount, uint8_t n);
int xact_atomic_fetch_sub_u64_multi(uint64_t **atomics, uint64_t *results, uint64_t amount, uint8_t n);

int xact_atomic_cas_u64_multi(uint64_t **atomics, const uint64_t *expected,
    const uint64_t *desired, uint8_t n);


#ifdef __cplusplus
}
#endif
