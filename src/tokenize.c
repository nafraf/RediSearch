/*
 * Copyright Redis Ltd. 2016 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "forward_index.h"
#include "stopwords.h"
#include "tokenize.h"
#include "toksep.h"
#include "rmalloc.h"
#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include "phonetic_manager.h"

typedef struct {
  RSTokenizer base;
  char *pos;
  Stemmer *stemmer;
} simpleTokenizer;

static void simpleTokenizer_Start(RSTokenizer *base, char *text, size_t len, uint16_t options) {
  simpleTokenizer *self = (simpleTokenizer *)base;
  TokenizerCtx *ctx = &base->ctx;
  ctx->text = text;
  ctx->options = options;
  ctx->len = len;
  ctx->empty_input = len == 0;
  self->pos = ctx->text;
}

// Normalization buffer
#define MAX_NORMALIZE_SIZE 128

/**
 * Normalizes text.
 * - s contains the raw token
 * - dst is the destination buffer which contains the normalized text
 * - len on input contains the length of the raw token. on output contains the
 * on output contains the length of the normalized token
 */
static char *DefaultNormalize_old(char *s, char *dst, size_t *len) {
  size_t origLen = *len;
  char *realDest = s;
  size_t dstLen = 0;

  // rune *rstr = NULL;
  // size_t nbegin;
  // t_len slen = 0;
  // size_t termLen;
  // rstr = strToFoldedRunes(s, &nbegin);
  // s = runesToStr(rstr, nbegin, &termLen);

  // strToFoldedStr(s, dst, len);

#define SWITCH_DEST()        \
  if (realDest != dst) {     \
    realDest = dst;          \
    memcpy(realDest, s, ii); \
  }
  // set to 1 if the previous character was a backslash escape
  int escaped = 0;
  for (size_t ii = 0; ii < origLen; ++ii) {
    // if (isupper(s[ii])) {
    //   SWITCH_DEST();
    //   realDest[dstLen++] = tolower(s[ii]);
    // } else
    if ((isblank(s[ii]) && !escaped) || iscntrl(s[ii])) {
      SWITCH_DEST();
    } else if (s[ii] == '\\' && !escaped) {
      SWITCH_DEST();
      escaped = 1;
      continue;
    } else {
      dst[dstLen++] = s[ii];
    }
    escaped = 0;
  }

  *len = dstLen;
  return dst;
}

static char *DefaultNormalize_unicode(char *s, char *dst, size_t *len) {
  size_t origLen = *len;

  ssize_t nu_len = nu_strlen(s, nu_utf8_read);
  if (nu_len > MAX_RUNESTR_LEN) {
    return NULL;
  }

  uint32_t decoded[nu_len + 1];
  decoded[nu_len] = 0;
  nu_readstr(s, decoded, nu_utf8_read);

  uint32_t unicode[nu_len + 1];
  unicode[nu_len] = 0;

  // set to 1 if the previous character was a backslash escape
  int escaped = 0;

  // position in the original string
  int char_pos = 0;
  int i = 0;
  for (i = 0; i < nu_len && char_pos < origLen; i++) {
    if ((isblank(s[char_pos]) && !escaped) || iscntrl(s[char_pos])) {
      unicode[i] = (uint32_t)decoded[i];
    } else if (s[char_pos] == '\\' && !escaped) {
      unicode[i] = (uint32_t)decoded[i];
      escaped = 1;
      char_pos++;
      continue;
    } else {
      unicode[i] = (rune)__fold(decoded[i]);
    }
    escaped = 0;

    if (decoded[i] < 0x100) {
      char_pos++;
    } else {
      char_pos += 2;
    }
  }
  // nu_writestr(unicode, dst, nu_utf8_write);
  nu_writenstr(unicode, i, dst, nu_utf8_write);
  *len = char_pos;
  return dst;
}

static char *DefaultNormalize(char *s, char *dst, size_t *len) {
  // char t1[MAX_NORMALIZE_SIZE];
  // char *x = DefaultNormalize_unicode(s, t1, len);
  return DefaultNormalize_unicode(s,dst, len);
  // return DefaultNormalize_old(s, dst, len);
}

// tokenize the text in the context
uint32_t simpleTokenizer_Next(RSTokenizer *base, Token *t) {
  TokenizerCtx *ctx = &base->ctx;
  simpleTokenizer *self = (simpleTokenizer *)base;
  while (self->pos != NULL) {
    // get the next token
    size_t origLen;
    char *tok = toksep(&self->pos, &origLen);
    // normalize the token
    size_t normLen = origLen;
    if (normLen > MAX_NORMALIZE_SIZE) {
      normLen = MAX_NORMALIZE_SIZE;
    }
    char normalized_s[MAX_NORMALIZE_SIZE];
    char *normBuf;
    if (ctx->options & TOKENIZE_NOMODIFY) {
      normBuf = normalized_s;
    } else {
      normBuf = tok;
    }

    char *normalized = DefaultNormalize(tok, normBuf, &normLen);

    // ignore tokens that turn into nothing, unless the whole string is empty.
    if ((normalized == NULL || normLen == 0) && !ctx->empty_input) {
      continue;
    }

    // skip stopwords
    if (!ctx->empty_input && StopWordList_Contains(ctx->stopwords, normalized, normLen)) {
      continue;
    }

    *t = (Token){.tok = normalized,
                 .tokLen = normLen,
                 .raw = tok,
                 .rawLen = origLen,
                 .pos = ++ctx->lastOffset,
                 .flags = Token_CopyStem,
                 .phoneticsPrimary = t->phoneticsPrimary};

    // if we support stemming - try to stem the word
    if (!(ctx->options & TOKENIZE_NOSTEM) && self->stemmer &&
          normLen >= RSGlobalConfig.iteratorsConfigParams.minStemLength) {
      size_t sl;
      const char *stem = self->stemmer->Stem(self->stemmer->ctx, tok, normLen, &sl);
      if (stem) {
        t->stem = stem;
        t->stemLen = sl;
      }
    }

    if ((ctx->options & TOKENIZE_PHONETICS) && normLen >= RSGlobalConfig.minPhoneticTermLen) {
      // VLA: eww
      if (t->phoneticsPrimary) {
        rm_free(t->phoneticsPrimary);
        t->phoneticsPrimary = NULL;
      }
      PhoneticManager_ExpandPhonetics(NULL, tok, normLen, &t->phoneticsPrimary, NULL);
    }

    return ctx->lastOffset;
  }

  return 0;
}

void simpleTokenizer_Free(RSTokenizer *self) {
  rm_free(self);
}

static void doReset(RSTokenizer *tokbase, Stemmer *stemmer, StopWordList *stopwords,
                    uint16_t opts) {
  simpleTokenizer *t = (simpleTokenizer *)tokbase;
  t->stemmer = stemmer;
  t->base.ctx.stopwords = stopwords;
  t->base.ctx.options = opts;
  t->base.ctx.lastOffset = 0;
  if (stopwords) {
    // Initially this function is called when we receive it from the mempool;
    // in which case stopwords is NULL.
    StopWordList_Ref(stopwords);
  }
}

RSTokenizer *NewSimpleTokenizer(Stemmer *stemmer, StopWordList *stopwords, uint16_t opts) {
  simpleTokenizer *t = rm_calloc(1, sizeof(*t));
  t->base.Free = simpleTokenizer_Free;
  t->base.Next = simpleTokenizer_Next;
  t->base.Start = simpleTokenizer_Start;
  t->base.Reset = doReset;
  t->base.Reset(&t->base, stemmer, stopwords, opts);
  return &t->base;
}

static mempool_t *tokpoolLatin_g = NULL;
static mempool_t *tokpoolCn_g = NULL;

static void *newLatinTokenizerAlloc() {
  return NewSimpleTokenizer(NULL, NULL, 0);
}
static void *newCnTokenizerAlloc() {
  return NewChineseTokenizer(NULL, NULL, 0);
}
static void tokenizerFree(void *p) {
  RSTokenizer *t = p;
  t->Free(t);
}

RSTokenizer *GetTokenizer(RSLanguage language, Stemmer *stemmer, StopWordList *stopwords) {
  if (language == RS_LANG_CHINESE) {
    return GetChineseTokenizer(stemmer, stopwords);
  } else {
    return GetSimpleTokenizer(stemmer, stopwords);
  }
}

RSTokenizer *GetChineseTokenizer(Stemmer *stemmer, StopWordList *stopwords) {
  if (!tokpoolCn_g) {
    mempool_options opts = {
        .initialCap = 16, .alloc = newCnTokenizerAlloc, .free = tokenizerFree};
    mempool_test_set_global(&tokpoolCn_g, &opts);
  }

  RSTokenizer *t = mempool_get(tokpoolCn_g);
  t->Reset(t, stemmer, stopwords, 0);
  return t;
}

RSTokenizer *GetSimpleTokenizer(Stemmer *stemmer, StopWordList *stopwords) {
  if (!tokpoolLatin_g) {
    mempool_options opts = {
        .initialCap = 16, .alloc = newLatinTokenizerAlloc, .free = tokenizerFree};
    mempool_test_set_global(&tokpoolLatin_g, &opts);
  }
  RSTokenizer *t = mempool_get(tokpoolLatin_g);
  t->Reset(t, stemmer, stopwords, 0);
  return t;
}

void Tokenizer_Release(RSTokenizer *t) {
  // In the future it would be nice to have an actual ID field or w/e, but for
  // now we can just compare callback pointers
  if (t->Next == simpleTokenizer_Next) {
    if (t->ctx.stopwords) {
      StopWordList_Unref(t->ctx.stopwords);
      t->ctx.stopwords = NULL;
    }
    mempool_release(tokpoolLatin_g, t);
  } else {
    mempool_release(tokpoolCn_g, t);
  }
}
