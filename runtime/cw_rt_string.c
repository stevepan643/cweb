#include "cw_rt_string.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct CW_RT_String_t {
  char *cstr;
  size_t length;
  size_t capacity;
};

static void cw_rt_string_ensure_capacity(CW_RT_String str,
                                         size_t min_capacity) {
  if (str->capacity >= min_capacity)
    return;
  size_t new_capacity = str->capacity;
  while (new_capacity < min_capacity)
    new_capacity *= 2;
  char *new_cstr = (char *)realloc(str->cstr, new_capacity);
  if (!new_cstr) {
    perror("cw_rt_string realloc failed");
    exit(EXIT_FAILURE);
  }
  str->cstr = new_cstr;
  str->capacity = new_capacity;
}

CW_RT_String cw_rt_string_new() {
  CW_RT_String s = (CW_RT_String)malloc(sizeof(struct CW_RT_String_t));
  s->capacity = 64;
  s->length = 0;
  s->cstr = (char *)malloc(s->capacity);
  s->cstr[0] = '\0';
  return s;
}

CW_RT_String cw_rt_string_new_from_cstr(const char *cstr) {
	if (!cstr) return NULL;
  CW_RT_String s = (CW_RT_String)malloc(sizeof(struct CW_RT_String_t));
  size_t len = strlen(cstr);
  s->capacity = len + 1;
  s->length = len;
  s->cstr = (char *)malloc(s->capacity);
  strcpy(s->cstr, cstr);
  return s;
}

CW_RT_String cw_rt_string_new_from_string(const CW_RT_String str) {
	if (!str || str->length == 0) return cw_rt_string_new();
  CW_RT_String s = (CW_RT_String)malloc(sizeof(struct CW_RT_String_t));
  s->capacity = str->capacity;
  s->length = str->length;
  s->cstr = (char *)malloc(s->capacity);
  strcpy(s->cstr, str->cstr);
  return s;
}

void cw_rt_string_delete(CW_RT_String str) {
	if (str == NULL) return;
  free(str->cstr);
  free(str);
}

void CW_RT_String_auto_free(CW_RT_String *str) { cw_rt_string_delete(*str); }

void cw_rt_string_delete_mult(CW_RT_String *arr, size_t cnt) {
  for (size_t i = 0; i < cnt; i++) {
    cw_rt_string_delete(arr[i]);
  }
}

const char *cw_rt_string_get_cstr(const CW_RT_String str) { return str->cstr; }
int cw_rt_string_get_length(const CW_RT_String str) { return (int)str->length; }

void cw_rt_string_append_str(CW_RT_String str, const CW_RT_String other) {
if (!str || !other) return;
  cw_rt_string_ensure_capacity(str, str->length + other->length + 1);
  strcpy(str->cstr + str->length, other->cstr);
  str->length += other->length;
}
void cw_rt_string_append_cstr(CW_RT_String str, const char *cstr) {
	if (!str || !cstr) return;
  size_t len = strlen(cstr);
  cw_rt_string_ensure_capacity(str, str->length + len + 1);
  strcpy(str->cstr + str->length, cstr);
  str->length += len;
}
void cw_rt_string_set_str(CW_RT_String str, const CW_RT_String other) {
if (!str || !other) return;
  cw_rt_string_ensure_capacity(str, other->length + 1);
  strcpy(str->cstr, other->cstr);
  str->length = other->length;
}
void cw_rt_string_set_cstr(CW_RT_String str, const char *cstr) {
	if (!str || !cstr) return;
  size_t len = strlen(cstr);
  cw_rt_string_ensure_capacity(str, len + 1);
  strcpy(str->cstr, cstr);
  str->length = len;
}

void cw_rt_string_insert_str(CW_RT_String str, size_t idx,
                             const CW_RT_String other) {
if (!str || !other) return;
  if (idx > str->length)
    idx = str->length;
  cw_rt_string_ensure_capacity(str, str->length + other->length + 1);
  memmove(str->cstr + idx + other->length, str->cstr + idx,
          str->length - idx + 1);
  memcpy(str->cstr + idx, other->cstr, other->length);
  str->length += other->length;
}
void cw_rt_string_insert_cstr(CW_RT_String str, size_t idx, const char *cstr) {
	if (!str || !cstr) return;
  size_t len = strlen(cstr);
  if (idx > str->length)
    idx = str->length;
  cw_rt_string_ensure_capacity(str, str->length + len + 1);
  memmove(str->cstr + idx + len, str->cstr + idx, str->length - idx + 1);
  memcpy(str->cstr + idx, cstr, len);
  str->length += len;
}

void cw_rt_string_remove(CW_RT_String str, size_t idx, size_t len) {
	if (!str) return;
  if (idx >= str->length)
    return;
  if (idx + len > str->length)
    len = str->length - idx;
  memmove(str->cstr + idx, str->cstr + idx + len, str->length - idx - len + 1);
  str->length -= len;
}
void cw_rt_string_clear(CW_RT_String str) {
	if (!str) return;
  str->length = 0;
  str->cstr[0] = '\0';
}

bool cw_rt_string_compare_str(const CW_RT_String str1,
                              const CW_RT_String str2) {
if (!str1 || !str2) return false;
  return strcmp(str1->cstr, str2->cstr) == 0;
}
bool cw_rt_string_compare_cstr(const CW_RT_String str, const char *cstr) {
	if (!str || !cstr) return false;
  return strcmp(str->cstr, cstr) == 0;
}

size_t cw_rt_string_find(const CW_RT_String str, const char *substr) {
	if (!str || !substr) return 0;
  char *pos = strstr(str->cstr, substr);
  if (!pos)
    return (size_t)-1;
  return (size_t)(pos - str->cstr);
}

static void cw_rt_string_replace_internal(CW_RT_String str, const char *old_sub,
                                          const char *new_sub,
                                          bool replace_all) {
  size_t old_len = strlen(old_sub);
  size_t new_len = strlen(new_sub);
  if (old_len == 0)
    return;

  char *pos = strstr(str->cstr, old_sub);
  if (!pos)
    return;

  size_t idx = pos - str->cstr;

  if (!replace_all) {
    size_t new_total_len = str->length - old_len + new_len;
    cw_rt_string_ensure_capacity(str, new_total_len + 1);

    memmove(str->cstr + idx + new_len, str->cstr + idx + old_len,
            str->length - idx - old_len + 1);
    memcpy(str->cstr + idx, new_sub, new_len);
    str->length = new_total_len;
    return;
  }

  size_t count = 0;
  char *tmp = str->cstr;
  while ((tmp = strstr(tmp, old_sub)) != NULL) {
    count++;
    tmp += old_len;
  }
  if (count == 0)
    return;

  size_t new_total_len = str->length + count * (new_len - old_len);
  cw_rt_string_ensure_capacity(str, new_total_len + 1);

  char *read = str->cstr;
  char *write = (char *)calloc(new_total_len + 1, 1);
  char *wptr = write;

  while (*read) {
    char *p = strstr(read, old_sub);
    if (p) {
      size_t n = p - read;
      memcpy(wptr, read, n);
      wptr += n;
      memcpy(wptr, new_sub, new_len);
      wptr += new_len;
      read = p + old_len;
    } else {
      strcpy(wptr, read);
      break;
    }
  }

  strcpy(str->cstr, write);
  memcpy(str->cstr, write, new_total_len);
str->cstr[new_total_len] = '\0';

  str->length = new_total_len;
  free(write);
}

void cw_rt_string_replace(const CW_RT_String str, const char *old_sub,
                          const char *new_sub) {
	if (!str || !old_sub || !new_sub ) return;
  cw_rt_string_replace_internal(str, old_sub, new_sub, false);
}

void cw_rt_string_replace_all(const CW_RT_String str, const char *old_sub,
                              const char *new_sub) {
	if (!str || !old_sub || !new_sub ) return;
  cw_rt_string_replace_internal(str, old_sub, new_sub, true);
}

void cw_rt_string_to_upper(CW_RT_String str) {
	if (!str) return;
  for (size_t i = 0; i < str->length; i++)
    str->cstr[i] = toupper(str->cstr[i]);
}
void cw_rt_string_to_lower(CW_RT_String str) {
	if (!str) return;
  for (size_t i = 0; i < str->length; i++)
    str->cstr[i] = tolower(str->cstr[i]);
}

void cw_rt_string_trim(CW_RT_String str) {
	if (!str) return;
  size_t start = 0;
  while (start < str->length && isspace((unsigned char)str->cstr[start]))
    start++;
  size_t end = str->length;
  while (end > start && isspace((unsigned char)str->cstr[end - 1]))
    end--;
  size_t new_len = end - start;
  if (start > 0)
    memmove(str->cstr, str->cstr + start, new_len);
  str->cstr[new_len] = '\0';
  str->length = new_len;
}

CW_RT_String cw_rt_string_substring(const CW_RT_String str, size_t start,
                                    size_t end) {
	if (!str) return NULL;
  if (start > str->length)
    start = str->length;
  if (end > str->length)
    end = str->length;
  if (start > end)
    start = end;
  CW_RT_String sub = cw_rt_string_new();
  size_t len = end - start;
  cw_rt_string_ensure_capacity(sub, len + 1);
  memcpy(sub->cstr, str->cstr + start, len);
  sub->cstr[len] = '\0';
  sub->length = len;
  return sub;
}

CW_RT_String *cw_rt_string_split(const CW_RT_String str, char delimiter,
                                 size_t *cnt) {
	if (!str | !cnt) return NULL;
  size_t count = 1;
  for (size_t i = 0; i < str->length; i++)
    if (str->cstr[i] == delimiter)
      count++;
  CW_RT_String *arr = (CW_RT_String *)malloc(sizeof(CW_RT_String) * count);
  size_t start = 0;
  size_t idx = 0;
  for (size_t i = 0; i <= str->length; i++) {
    if (i == str->length || str->cstr[i] == delimiter) {
      CW_RT_String s = cw_rt_string_new();
      size_t len = i - start;
      cw_rt_string_ensure_capacity(s, len + 1);
      memcpy(s->cstr, str->cstr + start, len);
      s->cstr[len] = '\0';
      s->length = len;
      arr[idx++] = s;
      start = i + 1;
    }
  }
  *cnt = count;
  return arr;
}

unsigned long cw_rt_string_hash(const CW_RT_String str) {
	if (!str) return 0;
  unsigned long hash = 5381;
  for (size_t i = 0; i < str->length; i++) {
    hash = ((hash << 5) + hash) + (unsigned char)str->cstr[i];
  }
  return hash;
}

