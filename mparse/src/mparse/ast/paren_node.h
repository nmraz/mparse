#pragma once

#include "mparse/ast/unary_node.h"

namespace mparse {

class paren_node : public ast_node_impl<paren_node, unary_node> {
public:
  paren_node() = default;
  paren_node(ast_node_ptr child);
};

}  // namespace mparse