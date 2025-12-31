#ifndef CW_RT_AUTO_FREE_H
#define CW_RT_AUTO_FREE_H

#define AUTO_FREE(TYPE) __attribute__((cleanup(TYPE##_auto_free))) TYPE

#endif /* CW_RT_AUTO_FREE_H */
