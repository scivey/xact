#pragma once
#include <stdint.h>
#include "xact/detail/asm/lockable_atomic_u64.h"
#ifdef __cplusplus
extern "C" {
#endif

struct xact_multi_atomic_op_argument_s {
  xact_lockable_atomic_u64_t *target;
  uint64_t arg1;
  uint64_t arg2;
};

typedef struct xact_multi_atomic_op_argument_s xact_multi_atomic_op_argument_t;

#ifdef __cplusplus
}
#endif
