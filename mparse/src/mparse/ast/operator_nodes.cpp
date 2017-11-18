#include "operator_nodes.h"

#include <algorithm>
#include <cassert>

namespace mparse {

unary_op_node::unary_op_node(unary_op_type type, ast_node* child) {
  set_type(type);
  set_child(child);
}

void unary_op_node::set_type(unary_op_type type) {
  type_ = type;
}


binary_op_node::binary_op_node(binary_op_type type, ast_node* lhs, ast_node* rhs) {
  set_type(type);
  set_lhs(lhs);
  set_rhs(rhs);
}

void binary_op_node::set_type(binary_op_type type) {
  type_ = type;
}

void binary_op_node::set_lhs(ast_node* lhs) {
  lhs_ = lhs;
}

void binary_op_node::set_rhs(ast_node* rhs) {
  rhs_ = rhs;
}

}  // namespace mparse