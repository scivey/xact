#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t xact_tsx_test_returned_abort_value(uint64_t code);
uint64_t xact_tsx_test_abort_explicit();
bool xact_sanity_test_eq_64(uint64_t x, uint64_t y);
uint64_t xact_sanity_test_add2_64(uint64_t x, uint64_t y);

#ifdef __cplusplus
}
#endif
