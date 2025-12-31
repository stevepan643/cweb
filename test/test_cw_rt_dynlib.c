#include "cw_rt_auto_free.h"
#include "cw_rt_dynlib.h"
#include "cw_unit_test.h"
#include <stdio.h>

TEST(load_and_unload) {
  AUTO_FREE(CW_RT_DynLib) lib = cw_rt_dynlib_load_from_path("./test/libtest.so");

  ASSERT(lib != NULL);
  ASSERT(cw_rt_dynlib_get_refcount(lib) == 1);

  cw_rt_dynlib_unload_dynlib(lib);
}

TEST(load_func_and_call) {
  AUTO_FREE(CW_RT_DynLib) lib = cw_rt_dynlib_load_from_path("./test/libtest.so");
  ASSERT(lib != NULL);

  AUTO_FREE(CW_RT_DynLib_FuncWrapper) func = cw_rt_dynlib_load_func(lib, "foo");
  ASSERT(func != NULL);
  ASSERT(cw_rt_dynlib_get_refcount(lib) == 2);

  int a = 3, b = 4;
  void *args[2] = {&a, &b};
  int res = cw_rt_dynlib_call_func(func, args, 2);
  ASSERT_EQ(res, 7);
}

TEST(error_cases) {
  ASSERT(cw_rt_dynlib_load_from_path(NULL) == NULL);
  ASSERT(cw_rt_dynlib_load_func(NULL, NULL) == NULL);
  ASSERT(cw_rt_dynlib_call_func(NULL, NULL, 0) == -1);
}

TEST_MAIN(load_and_unload, load_func_and_call, error_cases)
