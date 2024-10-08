/*
 * Copyright Redis Ltd. 2016 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include <string.h>
#include <sys/param.h>
#include <ctype.h>
#include "strings.h"
#include "alloc.h"

#include "hiredis/sds.h"

#include "rmalloc.h"

// RedisModuleString *RMUtil_CreateFormattedString(RedisModuleCtx *ctx, const char *fmt, ...) {
//     sds s = sdsempty();

//     va_list ap;
//     va_start(ap, fmt);
//     s = sdscatvprintf(s, fmt, ap);
//     va_end(ap);

//     RedisModuleString *ret = RedisModule_CreateString(ctx, (const char *)s, sdslen(s));
//     sdsfree(s);
//     return ret;
// }

int RMUtil_StringEquals(RedisModuleString *s1, RedisModuleString *s2) {

  const char *c1, *c2;
  size_t l1, l2;
  c1 = RedisModule_StringPtrLen(s1, &l1);
  c2 = RedisModule_StringPtrLen(s2, &l2);
  if (l1 != l2) return 0;

  return strncmp(c1, c2, l1) == 0;
}

int RMUtil_StringEqualsC(RedisModuleString *s1, const char *s2) {

  const char *c1;
  size_t l1, l2 = strlen(s2);
  c1 = RedisModule_StringPtrLen(s1, &l1);
  if (l1 != l2) return 0;

  return strncmp(c1, s2, l1) == 0;
}
int RMUtil_StringEqualsCaseC(RedisModuleString *s1, const char *s2) {

  const char *c1;
  size_t l1, l2 = strlen(s2);
  c1 = RedisModule_StringPtrLen(s1, &l1);
  if (l1 != l2) return 0;

  return strncasecmp(c1, s2, l1) == 0;
}

void RMUtil_StringToLower(RedisModuleString *s) {

  size_t l;
  char *c = (char *)RedisModule_StringPtrLen(s, &l);
  size_t i;
  for (i = 0; i < l; i++) {
    *c = tolower(*c);
    ++c;
  }
}

void RMUtil_StringToUpper(RedisModuleString *s) {
  size_t l;
  char *c = (char *)RedisModule_StringPtrLen(s, &l);
  size_t i;
  for (i = 0; i < l; i++) {
    *c = toupper(*c);
    ++c;
  }
}

void RMUtil_StringConvert(RedisModuleString **rs, const char **ss, size_t n, int options) {
  for (size_t ii = 0; ii < n; ++ii) {
    const char *p = RedisModule_StringPtrLen(rs[ii], NULL);
    if (options & RMUTIL_STRINGCONVERT_COPY) {
      p = rm_strdup(p);
    }
    ss[ii] = p;
  }
}

void print_rms(RedisModuleString *rms) {
    size_t n;
    const char *p = RedisModule_StringPtrLen(rms, &n);
    if (p) {
        printf("%.*s\n", (int) n, p);
    }
}
