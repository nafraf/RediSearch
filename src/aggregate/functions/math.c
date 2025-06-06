/*
 * Copyright (c) 2006-Present, Redis Ltd.
 * All rights reserved.
 *
 * Licensed under your choice of the Redis Source Available License 2.0
 * (RSALv2); or (b) the Server Side Public License v1 (SSPLv1); or (c) the
 * GNU Affero General Public License v3 (AGPLv3).
*/
#include "function.h"
#include <aggregate/expr/expression.h>
#include <math.h>

/* Template for single argument double to double math function */
#define NUMERIC_SIMPLE_FUNCTION(f)                                                               \
  static int mathfunc_##f(ExprEval *ctx, RSValue *argv, size_t argc, RSValue *result) {          \
    double d;                                                                                    \
    if (!RSValue_ToNumber(&argv[0], &d)) {                                                       \
      RSValue_SetNumber(result, NAN);                                                            \
      return EXPR_EVAL_OK;                                                                       \
    }                                                                                            \
    RSValue_SetNumber(result, f(d));                                                             \
    return EXPR_EVAL_OK;                                                                         \
  }

NUMERIC_SIMPLE_FUNCTION(log);
NUMERIC_SIMPLE_FUNCTION(floor);
NUMERIC_SIMPLE_FUNCTION(fabs);
NUMERIC_SIMPLE_FUNCTION(ceil);
NUMERIC_SIMPLE_FUNCTION(sqrt);
NUMERIC_SIMPLE_FUNCTION(log2);
NUMERIC_SIMPLE_FUNCTION(exp);

#define REGISTER_MATHFUNC(name, f) \
  RSFunctionRegistry_RegisterFunction(name, mathfunc_##f, RSValue_Number, 1, 1);

void RegisterMathFunctions() {
  REGISTER_MATHFUNC("log", log);
  REGISTER_MATHFUNC("floor", floor);
  REGISTER_MATHFUNC("abs", fabs);
  REGISTER_MATHFUNC("ceil", ceil);
  REGISTER_MATHFUNC("sqrt", sqrt);
  REGISTER_MATHFUNC("log2", log2);
  REGISTER_MATHFUNC("exp", exp);
}
