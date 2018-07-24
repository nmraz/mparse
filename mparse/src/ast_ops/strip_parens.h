#pragma once

#include "mparse/ast/ast_node.h"

namespace mparse::ast_ops {

void strip_parens(mparse::ast_node_ptr& node);

}  // namespace mparse::ast_ops