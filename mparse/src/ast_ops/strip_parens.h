#pragma once

#include "mparse/ast/ast_node.h"

namespace ast_ops {

void strip_parens(mparse::ast_node_ptr& node);

} // namespace ast_ops