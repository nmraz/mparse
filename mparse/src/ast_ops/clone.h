#pragma once

#include "mparse/ast.h"

namespace ast_ops {

mparse::ast_node_ptr clone(const mparse::ast_node& node);

} // namespace ast_ops