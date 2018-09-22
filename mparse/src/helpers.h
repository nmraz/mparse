#pragma once

#include "ast_ops/eval/scope.h"

void parse_vardefs(ast_ops::var_scope& vscope, int argc,
                   const char* const* argv);