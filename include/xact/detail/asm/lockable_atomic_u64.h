#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct xact_lockable_atomic_u64_s {
  volatile uint64_t value;
  volatile uint64_t tag;
} __attribute__((aligned(64)));

typedef struct xact_lockable_atomic_u64_s xact_lockable_atomic_u64_t;

#ifdef __cplusplus
}
#endif
