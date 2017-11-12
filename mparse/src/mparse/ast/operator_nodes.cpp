#include "operator_nodes.h"

#include <algorithm>
#include <cassert>

namespace mparse {

unary_op_node::unary_op_node(unary_op_type type, ast_node* child, std::size_t op_loc) {
  set_type(type);
  set_child(child);
  set_op_loc(op_loc);
}

void unary_op_node::set_type(unary_op_type type) {
  type_ = type;
}

void unary_op_node::set_op_loc(std::size_t op_loc) {
  op_loc_ = op_loc;
}

source_range unary_op_node::source_loc() const {
  source_range inner_loc = child()->source_loc();
  return { std::min(op_loc_, inner_loc.from()), std::max(op_loc_, inner_loc.to()) };
}


binary_op_node::binary_op_node(binary_op_type type, ast_node* lhs, ast_node* rhs, std::size_t op_loc) {
  set_type(type);
  set_lhs(lhs);
  set_rhs(rhs);
  set_op_loc(op_loc);
}

void binary_op_node::set_type(binary_op_type type) {
  type_ = type;
}

void binary_op_node::set_lhs(ast_node* lhs) {
  lhs_ = lhs;
  lhs_->set_parent(this);
}

void binary_op_node::set_rhs(ast_node* rhs) {
  rhs_ = rhs;
  rhs_->set_parent(this);
}

void binary_op_node::set_op_loc(std::size_t op_loc) {
  op_loc_ = op_loc;
}

source_range binary_op_node::source_loc() const {
  assert(
    op_loc_ >= lhs_->source_loc().from()
    && op_loc_ < rhs_->source_loc().to()
    && "Binary operator should be located between operands"
  );
  return { lhs_->source_loc().from(), rhs_->source_loc().to() };
}

}  // namespace mparse