#pragma once

#include "mparse/ast/ast_node_impl.h"
#include "mparse/ast/unary_node.h"

namespace mparse {

class paren_node : public ast_node_impl<paren_node, unary_node> {
public:
  constexpr paren_node() = default;
  explicit paren_node(ast_node_ptr child);
};

} // namespace mparse