/*
 * Copyright (c) 2006-Present, Redis Ltd.
 * All rights reserved.
 *
 * Licensed under your choice of the Redis Source Available License 2.0
 * (RSALv2); or (b) the Server Side Public License v1 (SSPLv1); or (c) the
 * GNU Affero General Public License v3 (AGPLv3).
*/

#include "libnu/libnu.h"
#include "rune_util.h"
#include "rmalloc.h"

#include <stdlib.h>
#include <string.h>

static uint32_t __fold(uint32_t runelike) {
  uint32_t lowered = 0;
  const char *map = 0;
  map = nu_tofold(runelike);
  if (!map) {
    return runelike;
  }
  nu_casemap_read(map, &lowered);
  return lowered;
}

rune runeFold(rune r) {
  return __fold((uint32_t)r);
}

static uint32_t __lower(uint32_t runelike) {
  uint32_t lowered = 0;
  const char *map = 0;
  map = nu_tolower(runelike);
  if (!map) {
    return runelike;
  }
  nu_casemap_read(map, &lowered);
  return lowered;
}

rune runeLower(rune r) {
  return __lower((uint32_t)r);
}

char *runesToStr(const rune *in, size_t len, size_t *utflen) {
  if (len > MAX_RUNESTR_LEN) {
    if (utflen) *utflen = 0;
    return NULL;
  }
  uint32_t unicode[len + 1];
  for (int i = 0; i < len; i++) {
    unicode[i] = (uint32_t)in[i];
  }
  unicode[len] = 0;

  *utflen = nu_bytelen(unicode, nu_utf8_write);
  char *ret = rm_calloc(1, *utflen + 1);

  nu_writestr(unicode, ret, nu_utf8_write);
  return ret;
}

/* convert string to runes, lower them and return the lowered runes */
rune *strToLowerRunes(const char *str, size_t *len) {

  // determine the length of the folded string
  ssize_t rlen = nu_strtransformlen(str, nu_utf8_read,
                                     nu_tolower, nu_casemap_read);
  if (rlen > MAX_RUNESTR_LEN) {
    if (len) *len = 0;
    return NULL;
  }

  uint32_t decoded[rlen + 1];
  decoded[rlen] = 0;
  nu_readstr(str, decoded, nu_utf8_read);

  rune *ret = rm_calloc(rlen + 1, sizeof(rune));
  const char *encoded_char = str;
  uint32_t codepoint;
  unsigned i = 0;
  for (ssize_t j = 0; j < rlen; j++) {
    // Read unicode codepoint from utf8 string
    encoded_char = nu_utf8_read(encoded_char, &codepoint);
    // Transform unicode codepoint to lower case
    const char *map = nu_tolower(codepoint);

    // Read the transformed codepoint and store it in the unicode buffer
    if (map != NULL) {
      uint32_t mu;
      while (1) {
        map = nu_casemap_read(map, &mu);
        if (mu == 0) {
          break;
        }
        ret[i] = mu;
        ++i;
      }
    }
    else {
      // If no transformation is needed, just copy the unicode codepoint
      ret[i] = codepoint;
      ++i;
    }
  }
  if (len) *len = rlen;

  return ret;
}

/* implementation is identical to that of strToRunes except for line where
 * __fold is called.
 * If the folded rune occupies more than 1 codepoint, only the first
 * is used, the rest are ignored. */
rune *strToSingleCodepointFoldedRunes(const char *str, size_t *len) {

  ssize_t rlen = nu_strlen(str, nu_utf8_read);
  if (rlen > MAX_RUNESTR_LEN) {
    if (len) *len = 0;
    return NULL;
  }

  uint32_t decoded[rlen + 1];
  decoded[rlen] = 0;
  nu_readstr(str, decoded, nu_utf8_read);

  rune *ret = rm_calloc(rlen + 1, sizeof(rune));
  for (int i = 0; i < rlen; i++) {
    uint32_t runelike = decoded[i];
    ret[i] = (rune)__fold(runelike);
  }
  if (len) *len = rlen;

  return ret;
}

rune *strToRunes(const char *str, size_t *len) {
  // Determine the length
  ssize_t rlen = nu_strlen(str, nu_utf8_read);
  if (rlen > MAX_RUNESTR_LEN) {
    if (len) *len = 0;
    return NULL;
  }

  rune *ret = rm_malloc((rlen + 1) * sizeof(rune));
  strToRunesN(str, strlen(str), ret);
  ret[rlen] = '\0';
  if (len) {
    *len = rlen;
  }
  return ret;
}

size_t strToRunesN(const char *src, size_t slen, rune *out) {
  const char *end = src + slen;
  size_t nout = 0;
  while (src < end) {
    uint32_t cp;
    src = nu_utf8_read(src, &cp);
    if (cp == 0) {
      break;
    }
    out[nout++] = (rune)cp;
  }
  return nout;
}

const rune *runenchr(const rune *r, size_t len, rune c) {
  size_t i = 0;
  for (; i < len; ++i) {
    if (r[i] == (rune)c) {
      break;
    }
  }
  return i == len ? NULL : r + i;
}
