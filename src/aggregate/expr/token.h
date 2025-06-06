/*
 * Copyright (c) 2006-Present, Redis Ltd.
 * All rights reserved.
 *
 * Licensed under your choice of the Redis Source Available License 2.0
 * (RSALv2); or (b) the Server Side Public License v1 (SSPLv1); or (c) the
 * GNU Affero General Public License v3 (AGPLv3).
*/
#ifndef RS_AGGREGATE_TOKEN_H_
#define RS_AGGREGATE_TOKEN_H_
#include <stdlib.h>
#include "expression.h"
/* A query-specific tokenizer, that reads symbols like quots, pipes, etc */
typedef struct {
  const char *raw;
  size_t len;
  char *pos;

  char *errorMsg;

  RSExpr *root;
  int ok;

} RSExprParseCtx;

/* A token in the process of parsing a query. Unlike the document tokenizer,  it
works iteratively and is not callback based.  */
typedef struct {
  const char *s;
  int len;
  int pos;
  double numval;
} RSExprToken;

#endif