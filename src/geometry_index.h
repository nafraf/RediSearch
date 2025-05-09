/*
 * Copyright Redis Ltd. 2016 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "geometry/geometry_types.h"
#include "search_ctx.h"
#include "index_iterator.h"

typedef struct GeometryQuery {
    GEOMETRY_FORMAT format;
    QueryType query_type;
    const FieldSpec *fs;
    const char *str;
    size_t str_len;
} GeometryQuery;

void GeometryQuery_Free(GeometryQuery *geomq);

GeometryIndex *OpenGeometryIndex(IndexSpec *spec, const FieldSpec *fs, bool create_if_missing);

RedisModuleString *fmtRedisGeometryIndexKey(const RedisSearchCtx *ctx, const HiddenString *field);

// Remove indexed data for the given document ID
void GeometryIndex_RemoveId(IndexSpec *spec, t_docId id);
