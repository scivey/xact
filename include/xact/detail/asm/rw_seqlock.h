#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// note: xact_rw_seqlock needs to be aligned properly,
// i.e. not across a cacheline boundary.
// alignment isn't specified here because it's being
// embedded in structures that already take care of alignment
struct xact_rw_seqlock_s {
  uint64_t value;
};

typedef struct xact_rw_seqlock_s xact_rw_seqlock_t;

#ifdef __cplusplus
}
#endif
