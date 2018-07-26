#pragma once

#include "ast_ops/eval/scope.h"
#include "ast_ops/eval/types.h"
#include "mparse/ast/ast_node.h"
#include <stdexcept>
#include <string_view>
#include <vector>

namespace ast_ops {

number eval(const mparse::ast_node* node, const var_scope& vscope, const func_scope& fscope);

}  // namespace ast_ops