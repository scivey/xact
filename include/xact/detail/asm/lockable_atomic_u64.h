#pragma once
#include <stdint.h>
#include "xact/detail/asm/rw_seqlock.h"
#ifdef __cplusplus
extern "C" {
#endif

struct xact_lockable_atomic_u64_s {
  uint64_t value;
  xact_rw_seqlock_t seqlock;
} __attribute__((aligned(16)));

typedef struct xact_lockable_atomic_u64_s xact_lockable_atomic_u64_t;

#ifdef __cplusplus
}
#endif
