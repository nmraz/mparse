#pragma once

#include "mparse/ast_impl.h"
#include <memory>
#include <string>
#include <vector>

namespace mparse {

class ast_node {
public:
  using derived_types = util::type_list<unary_node, binary_op_node, func_node,
                                        literal_node, id_node>;

  constexpr ast_node() = default;

  ast_node(const ast_node&) = delete;
  ast_node(ast_node&&) = delete;
  ast_node& operator=(const ast_node&) = delete;
  ast_node& operator=(ast_node&&) = delete;

  virtual ~ast_node() = 0 {}

private:
  template <typename Der, typename Base>
  friend class ast_node_impl;

  const void* id_ = nullptr;
};


class unary_node : public ast_node_impl<unary_node> {
public:
  using derived_types = util::type_list<abs_node, paren_node, unary_op_node>;

  constexpr unary_node() = default;

  ast_node* child() { return child_.get(); }
  const ast_node* child() const { return child_.get(); }

  void set_child(ast_node_ptr child);
  ast_node_ptr take_child();
  ast_node_ptr ref_child();

  ~unary_node() = 0 {}

private:
  ast_node_ptr child_;
};


class abs_node : public ast_node_impl<abs_node, unary_node> {
public:
  constexpr abs_node() = default;
  explicit abs_node(ast_node_ptr child);
};


class paren_node : public ast_node_impl<paren_node, unary_node> {
public:
  constexpr paren_node() = default;
  explicit paren_node(ast_node_ptr child);
};


enum class unary_op_type { plus, neg };

class unary_op_node : public ast_node_impl<unary_op_node, unary_node> {
public:
  unary_op_node() = default;
  unary_op_node(unary_op_type type, ast_node_ptr child);

  unary_op_type type() const { return type_; }
  void set_type(unary_op_type type);

private:
  unary_op_type type_;
};


enum class binary_op_type { add, sub, mult, div, pow };

class binary_op_node : public ast_node_impl<binary_op_node> {
public:
  binary_op_node() = default;
  binary_op_node(binary_op_type type, ast_node_ptr lhs, ast_node_ptr rhs);

  binary_op_type type() const { return type_; }
  void set_type(binary_op_type type);

  ast_node* lhs() { return lhs_.get(); }
  const ast_node* lhs() const { return lhs_.get(); }

  ast_node* rhs() { return rhs_.get(); }
  const ast_node* rhs() const { return rhs_.get(); }

  void set_lhs(ast_node_ptr lhs);
  ast_node_ptr take_lhs();
  ast_node_ptr ref_lhs();

  void set_rhs(ast_node_ptr rhs);
  ast_node_ptr take_rhs();
  ast_node_ptr ref_rhs();

private:
  binary_op_type type_;
  ast_node_ptr lhs_;
  ast_node_ptr rhs_;
};


class func_node : public ast_node_impl<func_node> {
public:
  using arg_list = std::vector<ast_node_ptr>;

  func_node() = default;
  func_node(std::string name, arg_list args);

  const std::string& name() const { return name_; }
  void set_name(std::string name);

  const arg_list& args() const { return args_; }
  arg_list& args() { return args_; }

private:
  std::string name_;
  arg_list args_;
};


class literal_node : public ast_node_impl<literal_node> {
public:
  constexpr literal_node() = default;
  constexpr explicit literal_node(double val) : val_(val) {}

  constexpr double val() const { return val_; }
  constexpr void set_val(double val) { val_ = val; }

private:
  double val_ = 0;
};


class id_node : public ast_node_impl<id_node> {
public:
  id_node() = default;
  explicit id_node(std::string name);

  const std::string& name() const { return name_; }
  void set_name(std::string name);

private:
  std::string name_;
};

} // namespace mparse