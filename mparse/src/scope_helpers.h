#pragma once

#include "ast_ops/eval/scope.h"

ast_ops::var_scope default_var_scope();
ast_ops::func_scope default_func_scope();

void parse_vardefs(ast_ops::var_scope& vscope,
                   int argc,
                   const char* const* argv);