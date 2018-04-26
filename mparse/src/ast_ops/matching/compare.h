#pragma once

#include "mparse/ast/ast_node.h"
#include <unordered_map>

namespace ast_ops::matching {

bool compare_exprs(const mparse::ast_node* first, const mparse::ast_node* second);

}  // namespace ast_ops::matching