#pragma once

#include "mparse/ast/ast_node.h"

namespace ast_ops {

void strip_parens(mparse::ast_node_ptr& node);

void canonicalize_ops(mparse::ast_node_ptr& node);
void canonicalize(mparse::ast_node_ptr& node);

void uncanonicalize(mparse::ast_node_ptr& node);

} // namespace ast_ops