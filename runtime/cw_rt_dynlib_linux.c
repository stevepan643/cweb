#include "cw_rt_dynlib.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct CW_RT_DynLib_t {
  void *handle;
  int refcount;
  CW_RT_DynLib *dependencies;
  size_t dep_count;
  int released;
};

struct CW_RT_DynLib_FuncWrapper_t {
  CW_RT_DynLib lib;
  void *func_ptr;
};

typedef const char **(*DynLibDepsFunc)(size_t *count);

static void cw_rt_dynlib_release(CW_RT_DynLib lib) {
  if (!lib || lib->released)
    return;

  if (--lib->refcount > 0)
    return;

  lib->released = 1;

  for (size_t i = 0; i < lib->dep_count; i++) {
    cw_rt_dynlib_release(lib->dependencies[i]);
  }
  free(lib->dependencies);

  if (lib->handle) {
    dlclose(lib->handle);
    lib->handle = NULL;
  }
  free(lib);
}

CW_RT_DynLib cw_rt_dynlib_load_from_path(const char *path) {
  if (!path)
    return NULL;

  CW_RT_DynLib lib = (CW_RT_DynLib)malloc(sizeof(*lib));
  if (!lib)
    return NULL;

  lib->handle = dlopen(path, RTLD_LAZY);
  if (!lib->handle) {
    fprintf(stderr, "dlopen failed for '%s': %s\n", path, dlerror());
    free(lib);
    return NULL;
  }

  lib->refcount = 1;
  lib->dependencies = NULL;
  lib->dep_count = 0;

  DynLibDepsFunc deps_func =
      (DynLibDepsFunc)dlsym(lib->handle, "_dynlib_get_dependencies");
  if (deps_func) {
    size_t dep_count = 0;
    const char **dep_paths = deps_func(&dep_count);
    if (dep_paths && dep_count > 0) {
      lib->dependencies =
          (CW_RT_DynLib *)calloc(dep_count, sizeof(CW_RT_DynLib));
      lib->dep_count = dep_count;
      for (size_t i = 0; i < dep_count; i++) {
        lib->dependencies[i] = cw_rt_dynlib_load_from_path(dep_paths[i]);
      }
    }
  }

  return lib;
}

size_t cw_rt_dynlib_get_refcount(CW_RT_DynLib lib) {
  return lib ? lib->refcount : 0;
}

void cw_rt_dynlib_unload_dynlib(CW_RT_DynLib lib) { cw_rt_dynlib_release(lib); }

CW_RT_DynLib_FuncWrapper cw_rt_dynlib_load_func(CW_RT_DynLib lib,
                                                const char *func_name) {
  if (!lib || !func_name || !lib->handle)
    return NULL;

  void *ptr = dlsym(lib->handle, func_name);
  if (!ptr) {
    fprintf(stderr, "dlsym failed for '%s': %s\n", func_name, dlerror());
    return NULL;
  }

  CW_RT_DynLib_FuncWrapper wrapper =
      (CW_RT_DynLib_FuncWrapper)malloc(sizeof(*wrapper));
  if (!wrapper)
    return NULL;

  wrapper->lib = lib;
  wrapper->func_ptr = ptr;

  lib->refcount++;

  return wrapper;
}

int cw_rt_dynlib_call_func(CW_RT_DynLib_FuncWrapper func, void **args,
                           int argc) {
  if (!func || !func->func_ptr)
    return -1;

  typedef int (*GenericFunc)(void **, int);
  GenericFunc f = (GenericFunc)func->func_ptr;
  return f(args, argc);
}

void CW_RT_DynLib_FuncWrapper_auto_free(CW_RT_DynLib_FuncWrapper *func) {
  if (!func || !*func)
    return;

  CW_RT_DynLib lib = (*func)->lib;
  free(*func);
  *func = NULL;

  cw_rt_dynlib_release(lib);
}

void CW_RT_DynLib_auto_free(CW_RT_DynLib *lib) {
  if (lib && *lib) {
    cw_rt_dynlib_release(*lib);
    *lib = NULL;
  }
}
