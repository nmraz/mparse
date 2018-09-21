#pragma once

#include "mparse/ast/ast_node_impl.h"
#include "mparse/ast/unary_node.h"

namespace mparse {

class abs_node : public ast_node_impl<abs_node, unary_node> {
public:
  constexpr abs_node() = default;
  abs_node(ast_node_ptr child);
};

} // namespace mparse