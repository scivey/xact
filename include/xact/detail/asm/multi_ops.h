#pragma once
#include <stdint.h>
#include "xact/detail/asm/multi_ops.h"
#ifdef __cplusplus
extern "C" {
#endif

int xact_multi_u64_load_2_mvcc(
  xact_multi_atomic_op_argument_t*,
  xact_multi_atomic_op_argument_t*
);

int xact_multi_u64_store_2_tsx(
  xact_multi_atomic_op_argument_t*,
  xact_multi_atomic_op_argument_t*
);

int xact_multi_u64_store_2_mvcc_with_locks_held(
  xact_multi_atomic_op_argument_t*,
  xact_multi_atomic_op_argument_t*
);

#ifdef __cplusplus
}
#endif
