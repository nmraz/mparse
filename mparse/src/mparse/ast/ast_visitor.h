#pragma once

#include <type_traits>

namespace mparse {

class ast_node;
class unary_node;
class paren_node;
class unary_op_node;
class binary_op_node;
class literal_node;

namespace impl {

template<template<typename> typename AddCv>
struct ast_visitor_cv {
  virtual void visit(AddCv<ast_node>&) {}
  virtual void visit(AddCv<unary_node>&) {}
  virtual void visit(AddCv<paren_node>&) {}
  virtual void visit(AddCv<unary_op_node>&) {}
  virtual void visit(AddCv<binary_op_node>&) {}
  virtual void visit(AddCv<literal_node>&) {}
};

template<typename T>
using identity = T;

}  // namespace impl

using ast_visitor = impl::ast_visitor_cv<impl::identity>;
using const_ast_visitor = impl::ast_visitor_cv<std::add_const_t>;

}  // namespace mparse