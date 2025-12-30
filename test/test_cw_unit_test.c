#include "cw_unit_test.h"

TEST(test_print) {
  printf("sss");
  ASSERT(1 == 1);
}

TEST(test_eq_pass) {
  int a = 2 + 3;
  ASSERT_EQ(a, 5);
}

TEST(test_fail) {
  int x = 0;
  ASSERT(x != 0);
}

TEST(test_eq_fail) { ASSERT_EQ(2, 3); }

TEST(test_skip_me) {
  printf("ppp");
  ASSERT(0);
}

TEST_MAIN(test_print, test_eq_pass, test_fail, test_eq_fail)
