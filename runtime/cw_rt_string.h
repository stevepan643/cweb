/**
 * @file  	runtime/cw_rt_string.h
 * @brief 	Abstraction of a dynamic string type in C.
 *
 * Provides APIs to manage and manipulate a heap-allocated string object,
 * as a safer alternative to raw C-style strings (char*).
 *
 * @version 	1.0
 * @author 	Steve Pan
 */

#ifndef CW_RT_STRING_H
#define CW_RT_STRING_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief 	Opaque handle of a runtime string object.
 */
typedef struct CW_RT_String_t *CW_RT_String;

/**
 * @brief 	Allocate an empty CW_RT_String.
 * @return 	A new string object, initially empty.
 */
CW_RT_String cw_rt_string_new();

/**
 * @brief 	Create a CW_RT_String from a C-style string.
 * @param cstr 	Null-terminated raw C string.
 * @return 	New string object containing a copy of @p cstr.
 */
CW_RT_String cw_rt_string_new_from_cstr(const char *cstr);

/**
 * @brief	Create a copy from an existing CW_RT_String.
 * @param str 	Source string.
 * @return 	New duplicated CW_RT_String.
 */
CW_RT_String cw_rt_string_new_from_string(const CW_RT_String str);

/**
 * @brief 	Destroy a CW_RT_String object.
 * @param str 	String to free. Safe to pass NULL.
 */
void cw_rt_string_delete(CW_RT_String str);

/**
 * %brief	Use for auto free.
 * @param str	String to free.
 * @see		cw_rt_auto_free.h
 */
void CW_RT_String_auto_free(CW_RT_String *str);

/**
 * @brief	Destroy multiple CW_RT_String objects.
 * @param str	Array of CW_RT_String handles.
 * @param cnt	Number of elements in @p str.
 */
void cw_rt_string_delete_mult(CW_RT_String *str, size_t cnt);

/**
 * @brief	Get internal C string pointer.
 * @param str	String object.
 * @return 	Const C string pointer (no need to free).
 */
const char *cw_rt_string_get_cstr(const CW_RT_String str);

/**
 * @brief	Get length of the string.
 * @param str	Target string.
 * @return 	Length in bytes (excluding null terminator).
 */
int cw_rt_string_get_length(const CW_RT_String str);

/**
 * @brief	Append another runtime string to @p str.
 */
void cw_rt_string_append_str(CW_RT_String str, const CW_RT_String other);

/**
 * @brief	Append a C-style string to @p str.
 */
void cw_rt_string_append_cstr(CW_RT_String str, const char *cstr);

/**
 * @brief	Replace content of @p str with another runtime string.
 */
void cw_rt_string_set_str(CW_RT_String str, const CW_RT_String other);

/**
 * @brief	Replace content of @p str with a C-style string.
 */
void cw_rt_string_set_cstr(CW_RT_String str, const char *cstr);

/**
 * @brief	Insert a runtime string at index @p idx.
 */
void cw_rt_string_insert_str(CW_RT_String str, size_t idx,
                             const CW_RT_String other);

/**
 * @brief	Insert a C-style string at index @p idx.
 */
void cw_rt_string_insert_cstr(CW_RT_String str, size_t idx, const char *cstr);

/**
 * @brief	Remove a substring at @p idx with length @p len.
 */
void cw_rt_string_remove(CW_RT_String str, size_t idx, size_t len);

/**
 * @brief	Clear the content of @p str (length becomes zero).
 */
void cw_rt_string_clear(CW_RT_String str);

/**
 * @brief	Compare two runtime strings.
 * @return 	true if equal, otherwise false.
 */
bool cw_rt_string_compare_str(const CW_RT_String str1, const CW_RT_String str2);

/**
 * @brief	Compare with a C-style string.
 * @return 	true if equal, otherwise false.
 */
bool cw_rt_string_compare_cstr(const CW_RT_String str, const char *cstr);

/**
 * @brief	Find substring position.
 * @return 	Index of match, or (size_t)-1 if not found.
 */
size_t cw_rt_string_find(const CW_RT_String str, const char *substr);

/**
 * @brief	Replace the first occurrence of @p old with @p new.
 */
void cw_rt_string_replace(const CW_RT_String str, const char *old,
                          const char *new);

/**
 * @brief	Replace all occurrences of @p old with @p new.
 */
void cw_rt_string_replace_all(const CW_RT_String str, const char *old,
                              const char *new);

/**
 * @brief	Convert characters to uppercase (ASCII only).
 */
void cw_rt_string_to_upper(CW_RT_String str);

/**
 * @brief	Convert characters to lowercase (ASCII only).
 */
void cw_rt_string_to_lower(CW_RT_String str);

/**
 * @brief	Trim leading and trailing whitespace.
 */
void cw_rt_string_trim(CW_RT_String str);

/**
 * @brief	Create a substring from [start, end).
 * @return	New allocated substring.
 */
CW_RT_String cw_rt_string_substring(const CW_RT_String str, size_t start,
                                    size_t end);

/**
 * @brief		Split the string by a delimiter.
 * @param str		Source string.
 * @param delimiter	Character delimiter.
 * @param cnt		Output count of resulting substrings.
 * @return		Array of CW_RT_String (free each + array itself).
 */
CW_RT_String *cw_rt_string_split(const CW_RT_String str, char delimiter,
                                 size_t *cnt);

/**
 * @brief	Compute a hash value for the string.
 * @return	Hash value.
 */
unsigned long cw_rt_string_hash(const CW_RT_String str);

/** @brief Convenient append interface using C11 _Generic. */
#define cw_rt_string_append(str, val)                                          \
  _Generic((val),                                                              \
      CW_RT_String: cw_rt_string_append_str,                                   \
      char *: cw_rt_string_append_cstr,                                        \
      const char *: cw_rt_string_append_cstr,\
      default: cw_rt_string_append_cstr)(str, val)

/** @brief Convenient set interface using C11 _Generic. */
#define cw_rt_string_set(str, val)                                             \
  _Generic((val),                                                              \
      CW_RT_String: cw_rt_string_set_str,                                      \
      char *: cw_rt_string_set_cstr,                                           \
      const char *: cw_rt_string_set_cstr,\
      default: cw_rt_string_set_cstr)(str, val)

/** @brief Convenient insert interface using C11 _Generic. */
#define cw_rt_string_insert(str, idx, val)                                     \
  _Generic((val),                                                              \
      CW_RT_String: cw_rt_string_insert_str,                                   \
      char *: cw_rt_string_insert_cstr,                                        \
      const char *: cw_rt_string_insert_cstr,\
      default: cw_rt_string_insert_cstr)(str, idx, val)

/** @brief Convenient compare interface using C11 _Generic. */
#define cw_rt_string_compare(str1, str2)                                       \
  _Generic((str2),                                                              \
      CW_RT_String: cw_rt_string_compare_str,                                  \
      char *: cw_rt_string_compare_cstr,                                        \
      const char *: cw_rt_string_compare_cstr,\
      default: cw_rt_string_compare_cstr)(str1, str2)

/** @brief Shorthand: create a CW_RT_String from a C string. */
#define STR(x) (cw_rt_string_new_from_cstr(x))

/** @brief Shorthand: get the raw C string from CW_RT_String. */
#define CSTR(x) (cw_rt_string_get_cstr(x))

#endif /* CW_RT_STRING_H */
