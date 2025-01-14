/*
 * Copyright Redis Ltd. 2016 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#ifndef RS_STRCONV_H_
#define RS_STRCONV_H_
#include <stdlib.h>
#include <limits.h>
#include <sys/errno.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <../trie/rune_util.h>
/* Strconv - common simple string conversion utils */

// Case insensitive string equal
#define STR_EQCASE(str, len, other) (len == strlen(other) && !strncasecmp(str, other, len))

// Case sensitive string equal
#define STR_EQ(str, len, other) (len == strlen(other) && !strncmp(str, other, len))

/* Parse string into int, returning 1 on success, 0 otherwise */
static int ParseInteger(const char *arg, long long *val) {

  char *e = NULL;
  errno = 0;
  *val = strtoll(arg, &e, 10);
  if ((errno == ERANGE && (*val == LONG_MAX || *val == LONG_MIN)) || (errno != 0 && *val == 0) ||
      *e != '\0') {
    *val = -1;
    return 0;
  }

  return 1;
}

/* Parse string into double, returning 1 on success, 0 otherwise */
static int ParseDouble(const char *arg, double *d, int sign) {
  char *e;
  errno = 0;

  // Simulate the behavior of glibc's strtod
  #if !defined(__GLIBC__)
  if (strcmp(arg, "") == 0) {
    *d = 0;
    return 1;
  }
  #endif

  *d = strtod(arg, &e);

  if ((errno == ERANGE && (*d == HUGE_VAL || *d == -HUGE_VAL)) || (errno != 0 && *d == 0) ||
      *e != '\0') {
    return 0;
  }

  if(sign == -1) {
    *d = -(*d);
  }

  return 1;
}

static int ParseBoolean(const char *arg, int *res) {
  if (STR_EQCASE(arg, strlen(arg), "true") || STR_EQCASE(arg, strlen(arg), "1")) {
    *res = 1;
    return 1;
  }

  if (STR_EQCASE(arg, strlen(arg), "false") || STR_EQCASE(arg, strlen(arg), "0")) {
    *res = 0;
    return 1;
  }

  return 0;
}

static char *strtolower(char *str) {
  char *p = str;
  while (*p) {
    *p = tolower(*p);
    p++;
  }
  return str;
}

static char *rm_strndup_unescape(const char *s, size_t len) {
  char *ret = rm_strndup(s, len);
  char *dst = ret;
  char *src = (char *)s;
  while (*src && len) {
    // unescape
    if (*src == '\\' && (ispunct(*(src+1)) || isspace(*(src+1)))) {
      ++src;
      --len;
      continue;
    }
    *dst = *src;
    ++dst;
    ++src;
    --len;
  }
  *dst = '\0';

  return ret;
}

// strndup + lowercase in one pass!
static char *rm_strdupcase_old(const char *s, size_t len) {
  char *ret = rm_strndup(s, len);
  char *dst = ret;
  char *src = dst;
  while (*src) {
    // unescape
    if (*src == '\\' && (ispunct(*(src+1)) || isspace(*(src+1)))) {
      ++src;
      continue;
    }
    *dst = tolower(*src);
    ++dst;
    ++src;

  }
  *dst = '\0';

  return ret;
}

// s is the input string in utf8
// len is the length of the input string in bytes
static char *rm_strdupcase_unicode(const char *s, size_t len) {
  char *ret = rm_strndup(s, len);
  ssize_t nu_len = nu_strlen(s, nu_utf8_read);
  if (nu_len > MAX_RUNESTR_LEN) {
    return NULL;
  }

  uint32_t decoded[nu_len + 1];
  decoded[nu_len] = 0;
  nu_readstr(s, decoded, nu_utf8_read);

  uint32_t unicode[nu_len + 1];
  unicode[nu_len] = 0;

  int char_pos = 0;
  int i = 0;
  for (i = 0; i < nu_len; i++) {
    // unescape
    if (s[char_pos] == '\\' && (ispunct(s[char_pos+1]) || isspace(s[char_pos+1]))) {
      char_pos++;
      continue;
    }

    unicode[i] = (rune)__fold(decoded[i]);

    if (decoded[i] < 0x100) {
      char_pos++;
    } else {
      char_pos += 2;
    }
  }

  nu_writenstr(unicode, i, ret, nu_utf8_write);
  return ret;
}

static char *rm_strdupcase(const char *s, size_t len) {
  return rm_strdupcase_unicode(s, len);
  // return rm_strdupcase_old(s, len);
}

#endif
