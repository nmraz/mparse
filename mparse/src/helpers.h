#pragma once

#include "ast_ops/eval/scope.h"
#include "ast_ops/eval/types.h"
#include "util/span.h"
#include <iostream>

void parse_vardefs(ast_ops::var_scope& vscope,
                   util::span<const char* const> argv);

std::ostream& print_number(std::ostream& stream, ast_ops::number num,
                           double round_prec = 10000000000000);