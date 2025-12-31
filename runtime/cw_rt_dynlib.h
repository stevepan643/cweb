#ifndef CW_RT_DYNLIB_H
#define CW_RT_DYNLIB_H

#include <stddef.h>

typedef struct CW_RT_DynLib_t *CW_RT_DynLib;
typedef struct CW_RT_DynLib_FuncWrapper_t *CW_RT_DynLib_FuncWrapper;

typedef int (*CW_RT_DynLib_GenericFunc)(void **args, int argc);

CW_RT_DynLib cw_rt_dynlib_load_from_path(const char *path);

void cw_rt_dynlib_unload_dynlib(CW_RT_DynLib lib);

CW_RT_DynLib_FuncWrapper cw_rt_dynlib_load_func(CW_RT_DynLib lib,
                                                const char *func_name);

size_t cw_rt_dynlib_get_refcount(CW_RT_DynLib lib);

int cw_rt_dynlib_call_func(CW_RT_DynLib_FuncWrapper func, void **args,
                           int argc);

void CW_RT_DynLib_auto_free(CW_RT_DynLib *lib);
void CW_RT_DynLib_FuncWrapper_auto_free(CW_RT_DynLib_FuncWrapper *func);

#endif /* CW_RT_DYNLIB_H */
