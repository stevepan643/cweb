#include "cw_rt_auto_free.h"
#include "cw_rt_string.h"
#include "cw_unit_test.h"
#include <stdio.h>
#include <string.h>

TEST(create_delete) {
  CW_RT_String s = cw_rt_string_new();
  ASSERT_EQ(cw_rt_string_get_length(s), 0);
  cw_rt_string_delete(s);

  CW_RT_String s2 = STR("hello");
  ASSERT_EQ(cw_rt_string_get_length(s2), 5);
  ASSERT(cw_rt_string_compare(s2, "hello"));

  CW_RT_String s3 = cw_rt_string_new_from_string(s2);
  ASSERT(cw_rt_string_compare(s3, s2));

  cw_rt_string_delete(s2);
  cw_rt_string_delete(s3);
}

TEST(auto_free) {
  AUTO_FREE(CW_RT_String) s = STR("auto");
  ASSERT(cw_rt_string_compare(s, "auto"));
}

TEST(compare) {
  AUTO_FREE(CW_RT_String) s1 = STR("abc");
  AUTO_FREE(CW_RT_String) s2 = cw_rt_string_new_from_string(s1);
  ASSERT(cw_rt_string_compare(s1, s2));
  ASSERT(!cw_rt_string_compare(s1, "abcd"));
}

TEST(append) {
  AUTO_FREE(CW_RT_String) s1 = STR("S1");
  AUTO_FREE(CW_RT_String) s2 = STR("S2");
  cw_rt_string_append(s1, s2);
  ASSERT(cw_rt_string_compare(s1, "S1S2"));

  cw_rt_string_append_cstr(s1, "");
  ASSERT(cw_rt_string_compare(s1, "S1S2"));
}

TEST(insert_remove) {
  AUTO_FREE(CW_RT_String) s = STR("hello");
  cw_rt_string_insert(s, 0, "X");
  ASSERT(cw_rt_string_compare(s, "Xhello"));
  cw_rt_string_insert(s, 100, "Y");
  ASSERT(cw_rt_string_compare(s, "XhelloY"));

  cw_rt_string_remove(s, 0, 1);
  ASSERT(cw_rt_string_compare(s, "helloY"));
  cw_rt_string_remove(s, 10, 1);
  ASSERT(cw_rt_string_compare(s, "helloY"));
  cw_rt_string_remove(s, 3, 100);
  ASSERT(cw_rt_string_compare(s, "hel"));
}

TEST(set) {
  AUTO_FREE(CW_RT_String) s = STR("abc");
  cw_rt_string_set(s, "xyz");
  ASSERT(cw_rt_string_compare(s, "xyz"));
  AUTO_FREE(CW_RT_String) s2 = STR("123");
  cw_rt_string_set(s, s2);
  ASSERT(cw_rt_string_compare(s, s2));
}

TEST(substring_find) {
  AUTO_FREE(CW_RT_String) s = STR("hello world");
  AUTO_FREE(CW_RT_String) sub = cw_rt_string_substring(s, 0, 5);
  ASSERT(cw_rt_string_compare(sub, "hello"));
  ASSERT_EQ(cw_rt_string_find(s, "world"), 6);
  ASSERT_EQ(cw_rt_string_find(s, "xxx"), (size_t)-1);
}

TEST(split_tests) {
  AUTO_FREE(CW_RT_String) s = STR("a,b,c");
  size_t cnt;
  CW_RT_String *arr = cw_rt_string_split(s, ',', &cnt);
  ASSERT_EQ(cnt, 3);
  ASSERT(cw_rt_string_compare(arr[0], "a"));
  ASSERT(cw_rt_string_compare(arr[1], "b"));
  ASSERT(cw_rt_string_compare(arr[2], "c"));
  for (size_t i = 0; i < cnt; i++)
    cw_rt_string_delete(arr[i]);
  free(arr);
}

TEST(replace_tests) {
  AUTO_FREE(CW_RT_String) s = STR("foo bar foo");
  cw_rt_string_replace(s, "foo", "baz");
  ASSERT(cw_rt_string_compare(s, "baz bar foo"));
  cw_rt_string_replace_all(s, "foo", "qux");
  ASSERT(cw_rt_string_compare(s, "baz bar qux"));
}

TEST(case_trim_tests) {
  AUTO_FREE(CW_RT_String) s = STR(" AbC ");
  cw_rt_string_to_upper(s);
  ASSERT(cw_rt_string_compare(s, " ABC "));
  cw_rt_string_to_lower(s);
  ASSERT(cw_rt_string_compare(s, " abc "));
  cw_rt_string_trim(s);
  ASSERT(cw_rt_string_compare(s, "abc"));
}

TEST(hash_test) {
  AUTO_FREE(CW_RT_String) s1 = STR("abc");
  AUTO_FREE(CW_RT_String) s2 = STR("abc");
  AUTO_FREE(CW_RT_String) s3 = STR("def");
  ASSERT_EQ(cw_rt_string_hash(s1), cw_rt_string_hash(s2));
  ASSERT(cw_rt_string_hash(s1) != cw_rt_string_hash(s3));
}

TEST(error_cases) {
  cw_rt_string_append_cstr(NULL, "a");
  cw_rt_string_append_cstr(NULL, NULL);
  cw_rt_string_append_str(NULL, NULL);
  cw_rt_string_set_cstr(NULL, "a");
  cw_rt_string_set_cstr(NULL, NULL);
  cw_rt_string_set_str(NULL, NULL);
  cw_rt_string_insert_cstr(NULL, 0, "a");
  cw_rt_string_insert_cstr(NULL, 0, NULL);
  cw_rt_string_insert_str(NULL, 0, NULL);
  cw_rt_string_compare(NULL, NULL);
  cw_rt_string_compare(NULL, "a");
  cw_rt_string_compare(STR("a"), NULL);
  cw_rt_string_find(NULL, NULL);
  cw_rt_string_replace(NULL, NULL, NULL);
  cw_rt_string_replace_all(NULL, NULL, NULL);
  cw_rt_string_substring(NULL, 0, 1);
  cw_rt_string_split(NULL, ',', NULL);
  cw_rt_string_to_upper(NULL);
  cw_rt_string_to_lower(NULL);
  cw_rt_string_trim(NULL);
}

TEST_MAIN(create_delete, auto_free, compare, append, insert_remove, set,
          substring_find, split_tests, replace_tests, case_trim_tests,
          hash_test, error_cases)
