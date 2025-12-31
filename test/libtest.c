#include <stddef.h>

int foo(void **args, int argc) {
  int a = *((int *)args[0]);
  int b = *((int *)args[1]);
  return a + b;
}

const char **_dynlib_get_dependencies(size_t *count) {
  *count = 0;
  return NULL;
}
