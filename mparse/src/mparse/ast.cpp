#include "ast.h"

namespace mparse {

ast_node::~ast_node() { // definition provided because base dtors are always
                        // called (even when pure virtual)
}

void ast_node::apply_visitor(ast_visitor& vis) {
  vis.visit(*this);
}

void ast_node::apply_visitor(const_ast_visitor& vis) const {
  vis.visit(*this);
}


void unary_node::set_child(ast_node_ptr child) {
  child_ = std::move(child);
}

ast_node_ptr unary_node::take_child() {
  return std::move(child_);
}

ast_node_ptr unary_node::ref_child() {
  return child_;
}


abs_node::abs_node(ast_node_ptr child) {
  set_child(std::move(child));
}


paren_node::paren_node(ast_node_ptr child) {
  set_child(std::move(child));
}


unary_op_node::unary_op_node(unary_op_type type, ast_node_ptr child)
    : type_(type) {
  set_child(std::move(child));
}

void unary_op_node::set_type(unary_op_type type) {
  type_ = type;
}


binary_op_node::binary_op_node(binary_op_type type, ast_node_ptr lhs,
                               ast_node_ptr rhs)
    : type_(type), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

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


func_node::func_node(std::string name, arg_list args)
    : name_(std::move(name)), args_(std::move(args)) {}

void func_node::set_name(std::string name) {
  name_ = name;
}


id_node::id_node(std::string name) {
  set_name(std::move(name));
}

void id_node::set_name(std::string name) {
  name_ = std::move(name);
}

} // namespace mparse