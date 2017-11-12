#pragma once

#include "mparse/ast/ast_node.h"

namespace mparse {

class unary_node : public ast_node_impl<unary_node> {
public:
  ast_node* child() { return child_; }
  const ast_node* child() const { return child_; }

  void set_child(ast_node* child);

private:
  ast_node* child_;
};

}  // namespace mparse
