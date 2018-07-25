#pragma once

#include "ast_ops/eval/exceptions.h"
#include "mparse/error.h"
#include "mparse/source_map.h"
#include <string_view>

void handle_syntax_error(const mparse::syntax_error& err, std::string_view input);
void handle_math_error(const ast_ops::eval_error& err, const mparse::source_map& smap,
  std::string_view input);