#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int xact_generalized_cas_op_tsx_impl(void *preconditions, uint64_t numPreconditions,
    void *operations, uint64_t numOperations);

int xact_generalized_cas_op_tsx_with_retries(void *preconditions, uint64_t numPreconditions,
    void *operations, uint64_t numOperations, uint64_t nRetries);

int xact_generalized_cas_op_with_locks_acquired(void *preconditions, uint64_t numPreconditions,
    void *operations, uint64_t numOperations);


#ifdef __cplusplus
}
#endif
