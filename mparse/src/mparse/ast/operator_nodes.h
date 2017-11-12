#pragma once

#include "mparse/ast/unary_node.h"

namespace mparse {

enum class unary_op_type {
  plus,
  neg
};

class unary_op_node : public ast_node_impl<unary_op_node, unary_node> {
public:
  unary_op_node() = default;
  unary_op_node(unary_op_type type, ast_node* child, std::size_t op_loc);

  unary_op_type type() const { return type_; }
  void set_type(unary_op_type type);

  std::size_t op_loc() const { return op_loc_; }
  void set_op_loc(std::size_t op_loc);

  source_range source_loc() const override;

private:
  unary_op_type type_;
  std::size_t op_loc_;
};


enum class binary_op_type {
  add,
  sub,
  mult,
  div,
  pow
};

class binary_op_node : public ast_node_impl<binary_op_node> {
public:
  binary_op_node() = default;
  binary_op_node(binary_op_type type, ast_node* lhs, ast_node* rhs, std::size_t op_loc);

  binary_op_type type() const { return type_; }
  void set_type(binary_op_type type);

  ast_node* lhs() { return lhs_; }
  const ast_node* lhs() const { return lhs_; }

  ast_node* rhs() { return rhs_; }
  const ast_node* rhs() const { return rhs_; }

  void set_lhs(ast_node* lhs);
  void set_rhs(ast_node* rhs);

  std::size_t op_loc() const { return op_loc_; }
  void set_op_loc(std::size_t op_loc);

  source_range source_loc() const override;

private:
  binary_op_type type_;
  ast_node* lhs_;
  ast_node* rhs_;
  std::size_t op_loc_;
};

}  // namespace mparse
