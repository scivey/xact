#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int xact_asm_dcas_u64(
  uint64_t *target1, uint64_t *target1_expected, uint64_t target1_desired,
  uint64_t *target2, uint64_t *target2_expected, uint64_t target2_desired
);

int xact_asm_3cas_u64(
  uint64_t *target1, uint64_t *target1_expected, uint64_t target1_desired,
  uint64_t *target2, uint64_t *target2_expected, uint64_t target2_desired,
  uint64_t *target3, uint64_t *target3_expected, uint64_t target3_desired
);

bool xact_asm_scas_u64(
  uint64_t *target, uint64_t *target_expected, uint64_t target_desired
);

bool xact_eq_test(uint64_t x, uint64_t* y);
uint64_t xact_add_test(uint64_t* x, uint64_t* y);

#ifdef __cplusplus
}
#endif
