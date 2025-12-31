#ifndef CW_UNIT_TEST_H
#define CW_UNIT_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_BOLD "\033[1m"

typedef void (*TestFunc)(void);

typedef struct TestEntry {
  const char *name;
  TestFunc func;
  int run;
  int failed;
  struct TestEntry *next;
} TestEntry;

static TestEntry *test_list_head = NULL;

static void add_test(const char *name, TestFunc func) {
  TestEntry *entry = malloc(sizeof(TestEntry));
  if (!entry) {
    perror("malloc");
    exit(1);
  }
  entry->name = name;
  entry->func = func;
  entry->run = 0;
  entry->failed = 0;
  entry->next = test_list_head;
  test_list_head = entry;
}

#define TEST(name)                                                             \
  static void test_##name(void);                                               \
  __attribute__((constructor)) static void register_##name(void) {             \
    add_test(#name, test_##name);                                              \
  }                                                                            \
  static void test_##name(void)

#define ASSERT(expr)                                                           \
  do {                                                                         \
    if (!(expr)) {                                                             \
      fprintf(stderr,                                                          \
              COLOR_RED COLOR_BOLD "%s:%d: Assertion `%s` failed" COLOR_RESET, \
              __FILE__, __LINE__, #expr);                                      \
      if (current_test)                                                        \
        current_test->failed = 1;                                              \
    }                                                                          \
  } while (0)

#define ASSERT_EQ(a, b)                                                        \
  do {                                                                         \
    if ((a) != (b)) {                                                          \
      fprintf(stderr,                                                          \
              COLOR_RED COLOR_BOLD "%s:%d: Assertion `%s == %s` "              \
                                   "failed (%lld != %lld)" COLOR_RESET,        \
              __FILE__, __LINE__, #a, #b, (long long)(a), (long long)(b));     \
      if (current_test)                                                        \
        current_test->failed = 1;                                              \
    }                                                                          \
  } while (0)

static TestEntry *current_test = NULL;

static int run_tests(void) {
  int passed = 0, skipped = 0, failed = 0, total = 0;
  TestEntry *p = test_list_head;
  while (p) {
    total++;
    p = p->next;
  }

  printf(COLOR_BOLD "==================== Running %d tests "
                    "====================\n" COLOR_RESET,
         total);

  p = test_list_head;
  while (p) {
    if (p->run) {
      printf(COLOR_BLUE "Running test %s..." COLOR_RESET, p->name);
      fflush(stdout);

      FILE *old_stdout = stdout;
      stdout = fopen("/dev/null", "w");
      if (!stdout) {
        perror("fopen");
        exit(1);
      }

      current_test = p;
      p->func();
      current_test = NULL;

      fclose(stdout);
      stdout = old_stdout;

      if (p->failed) {
        printf(COLOR_RED " [FAIL]\n" COLOR_RESET);
        failed++;
      } else {
        printf(COLOR_GREEN " [PASS]\n" COLOR_RESET);
        passed++;
      }
    } else {
      printf(COLOR_YELLOW "Skipping test %s [SKIPPED]\n" COLOR_RESET, p->name);
      skipped++;
    }
    p = p->next;
  }

  printf(COLOR_BOLD
         "==================== Summary ====================\n" COLOR_RESET);
  printf("Passed %s%d%s, Failed %s%d%s, Skipped %s%d%s, Total %d\n",
         COLOR_GREEN, passed, COLOR_RESET, COLOR_RED, failed, COLOR_RESET,
         COLOR_YELLOW, skipped, COLOR_RESET, total);
  if (failed != 0)
    return -1;
  else
    return 0;
}

#define TEST_MAIN(...)                                                         \
  int main(void) {                                                             \
    const char *all_names = #__VA_ARGS__;                                      \
    char name_buf[128];                                                        \
    size_t start = 0, len = 0;                                                 \
    for (size_t i = 0;; i++) {                                                 \
      char c = all_names[i];                                                   \
      if (c == ',' || c == '\0') {                                             \
        len = i - start;                                                       \
        if (len > 0) {                                                         \
          if (len >= sizeof(name_buf))                                         \
            len = sizeof(name_buf) - 1;                                        \
          memcpy(name_buf, all_names + start, len);                            \
          name_buf[len] = '\0';                                                \
          char *s = name_buf;                                                  \
          while (*s == ' ')                                                    \
            s++;                                                               \
          char *e = name_buf + len - 1;                                        \
          while (e > s && (*e == ' ' || *e == '\t'))                           \
            *e-- = '\0';                                                       \
          TestEntry *t;                                                        \
          for (t = test_list_head; t; t = t->next) {                           \
            if (strcmp(t->name, s) == 0)                                       \
              t->run = 1;                                                      \
          }                                                                    \
        }                                                                      \
        if (c == '\0')                                                         \
          break;                                                               \
        start = i + 1;                                                         \
      }                                                                        \
    }                                                                          \
    return run_tests();                                                        \
  }

#endif /* CW_UNIT_TEST_H */
