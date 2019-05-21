#pragma once

#include "mparse/ast/ast_node_impl.h"

namespace mparse {

class abs_node;
class paren_node;
class unary_op_node;

class unary_node : public ast_node_impl<unary_node> {
public:
  using derived_types = util::type_list<abs_node, paren_node, unary_op_node>;

  constexpr unary_node() = default;

  ast_node* child() { return child_.get(); }
  const ast_node* child() const { return child_.get(); }

  void set_child(ast_node_ptr child);
  ast_node_ptr take_child();
  ast_node_ptr ref_child();

private:
  ast_node_ptr child_;
};

} // namespace mparse
