#pragma once

#include "ast_ops/eval/eval_scope.h"
#include "mparse/ast/ast_node.h"
#include <stdexcept>
#include <string_view>
#include <vector>

namespace ast_ops {

enum class eval_errc {
  unknown,
  div_by_zero,
  bad_pow,
  unbound_var
};

class eval_error : public std::runtime_error {
public:
  eval_error(std::string_view what, eval_errc code, const mparse::ast_node* node);

  eval_errc code() const { return code_; }
  const mparse::ast_node* node() const { return node_; }

private:
  eval_errc code_;
  const mparse::ast_node* node_;
};

double eval(const mparse::ast_node* node, const eval_scope& scope);

}  // namespace ast_ops