#pragma once

#include "ast_ops/eval/scope.h"
#include "mparse/ast/ast_node.h"
#include <stdexcept>
#include <string_view>
#include <vector>

namespace ast_ops {

double eval(const mparse::ast_node* node, const var_scope& scope);

}  // namespace ast_ops