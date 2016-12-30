#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void xact_mfence();
void xact_busy_wait(uint64_t n_pauses);

#ifdef __cplusplus
}
#endif
