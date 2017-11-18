#pragma once

#include "mparse/ast/abstract_syntax_tree.h"
#include "mparse/source_range.h"
#include <stdexcept>
#include <string_view>
#include <vector>

enum class eval_errc {
  div_by_zero,
  bad_pow
};

class eval_error : public std::runtime_error {
public:
  eval_error(std::string_view what, eval_errc code, const mparse::ast_node& node);

  eval_errc code() const { return code_; }
  const mparse::ast_node& node() const { return node_; }

private:
  eval_errc code_;
  const mparse::ast_node& node_;
};

double eval(const mparse::abstract_syntax_tree& ast);