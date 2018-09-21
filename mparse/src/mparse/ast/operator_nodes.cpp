#include "operator_nodes.h"

#include <algorithm>
#include <cassert>
#include <utility>

namespace mparse {

unary_op_node::unary_op_node(unary_op_type type, ast_node_ptr child) {
  set_type(type);
  set_child(std::move(child));
}

void unary_op_node::set_type(unary_op_type type) {
  type_ = type;
}


binary_op_node::binary_op_node(binary_op_type type,
                               ast_node_ptr lhs,
                               ast_node_ptr rhs) {
  set_type(type);
  set_lhs(std::move(lhs));
  set_rhs(std::move(rhs));
}

void binary_op_node::set_type(binary_op_type type) {
  type_ = type;
}


void binary_op_node::set_lhs(ast_node_ptr lhs) {
  lhs_ = std::move(lhs);
}

ast_node_ptr binary_op_node::take_lhs() {
  return std::move(lhs_);
}

ast_node_ptr binary_op_node::ref_lhs() {
  return lhs_;
}


void binary_op_node::set_rhs(ast_node_ptr rhs) {
  rhs_ = std::move(rhs);
}

ast_node_ptr binary_op_node::take_rhs() {
  return std::move(rhs_);
}

ast_node_ptr binary_op_node::ref_rhs() {
  return rhs_;
}

} // namespace mparse