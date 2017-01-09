#pragma once
#include <stdint.h>
#include "xact/detail/asm/rw_seqlock.h"
#ifdef __cplusplus
extern "C" {
#endif

int xact_rw_seqlock_init(xact_rw_seqlock_t*);

int xact_rw_seqlock_write_unlock(xact_rw_seqlock_t*);
int xact_rw_seqlock_write_lock(xact_rw_seqlock_t*);
int xact_rw_seqlock_try_write_lock(xact_rw_seqlock_t*);
bool xact_rw_seqlock_is_write_locked_from_uint64_t(uint64_t);
bool xact_rw_seqlock_is_write_locked(xact_rw_seqlock_t*);

int xact_rw_seqlock_read_unlock(xact_rw_seqlock_t*);
int xact_rw_seqlock_read_lock(xact_rw_seqlock_t*);
int xact_rw_seqlock_try_read_lock(xact_rw_seqlock_t*);
bool xact_rw_seqlock_is_read_locked_from_uint64_t(uint64_t);
bool xact_rw_seqlock_is_read_locked(xact_rw_seqlock_t*);

bool xact_rw_seqlock_is_locked(xact_rw_seqlock_t*);

int xact_rw_seqlock_get_version_from_uint64_t(uint64_t);
int xact_rw_seqlock_get_version(xact_rw_seqlock_t*);

uint64_t xact_rw_seqlock_get_raw_value(xact_rw_seqlock_t*);
uint64_t xact_rw_seqlock_util_make_write_locked_update_from_uint64_t(uint64_t);

#ifdef __cplusplus
}
#endif
