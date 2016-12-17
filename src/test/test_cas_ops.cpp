#include <gtest/gtest.h>
#include "xact/cas_ops.h"
#include "xact/asm/core.h"


using u64_params_t = xact::IntegralCASParams<uint64_t>;

TEST(TestDCasU64, TestHappy1) {
  uint64_t target1 {10}, expected1 = 10;
  uint64_t target2 {93}, expected2 = 93;

  uint64_t desired1 = 17;
  uint64_t desired2 = 502;

  auto params1 = u64_params_t()
    .target(&target1)
    .expected(&expected1)
    .desired(desired1);

  auto params2 = u64_params_t()
    .target(&target2)
    .expected(&expected2)
    .desired(desired2);

  auto outcome = xact::doubleCAS_U64(params1, params2);
  EXPECT_TRUE(outcome);
  EXPECT_EQ(17, target1);
  EXPECT_EQ(502, target2);
  EXPECT_EQ(10, expected1);
  EXPECT_EQ(93, expected2);
  EXPECT_EQ(17, desired1);
  EXPECT_EQ(502, desired2);
}


TEST(TestDCasU64, TestSad1) {
  uint64_t target1 {10}, expected1 = 10;
  uint64_t target2 {93}, expected2 = 93;

  uint64_t desired1 = 17;
  uint64_t desired2 = 502;

  auto params1 = u64_params_t()
    .target(&target1)
    .expected(&expected1)
    .desired(desired1);

  auto params2 = u64_params_t()
    .target(&target2)
    .expected(&expected2)
    .desired(desired2);

  target1 = 3003;

  auto outcome = xact::doubleCAS_U64(params1, params2);
  EXPECT_FALSE(outcome);
  EXPECT_EQ(3003, target1);
  EXPECT_EQ(93, target2);
  EXPECT_EQ(3003, expected1);
  EXPECT_EQ(93, expected2);
}

TEST(TestSingleCasU64, TestHappy1) {
  uint64_t target1 {10}, expected1 = 10;
  uint64_t desired1 = 17;

  auto params1 = u64_params_t()
    .target(&target1)
    .expected(&expected1)
    .desired(desired1);

  auto outcome = xact::singleCAS_U64(params1);
  EXPECT_TRUE(outcome);
  EXPECT_EQ(17, target1);
  EXPECT_EQ(10, expected1);
}


TEST(TestEquality, TestSanity) {
  uint64_t x = 5, y = 5;
  EXPECT_TRUE(xact_eq_test(x, &y));
  x = 7;
  EXPECT_FALSE(xact_eq_test(x, &y));
  y = 7;
  EXPECT_TRUE(xact_eq_test(x, &y));
  y = 9;
  EXPECT_FALSE(xact_eq_test(x, &y));  
}

TEST(TestAdd, TestSanity) {
  uint64_t x = 5, y = 5;
  EXPECT_EQ(10, xact_add_test(&x, &y));
  y = 7;
  EXPECT_EQ(12, xact_add_test(&x, &y));

}
