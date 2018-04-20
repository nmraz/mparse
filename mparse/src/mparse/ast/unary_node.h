#pragma once

#include "mparse/ast/ast_node.h"

namespace mparse {

class unary_node : public ast_node_impl<unary_node> {
public:
  constexpr unary_node() = default;

  ast_node* child() { return child_.get(); }
  const ast_node* child() const { return child_.get(); }

  void set_child(ast_node_ptr child);
  ast_node_ptr take_child();

private:
  ast_node_ptr child_;
};

}  // namespace mparse
