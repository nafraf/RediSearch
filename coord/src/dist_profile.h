/*
 * Copyright Redis Ltd. 2016 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "reply.h"
#include "rmr/reply.h"

typedef struct PrintShardProfile_ctx {
  MRReply **replies;
  int count;
  bool isSearch;
} PrintShardProfile_ctx;
