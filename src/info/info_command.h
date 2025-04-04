/*
 * Copyright Redis Ltd. 2016 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */
#pragma once

#include "redismodule.h"
#ifdef __cplusplus
extern "C" {
#endif

int IndexInfoCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int IndexObfuscatedInfo(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
#ifdef __cplusplus
}
#endif
