#pragma once

#include "util/meta.h"
#include <type_traits>

namespace mparse {

class ast_node;
class unary_node;
class paren_node;
class abs_node;
class unary_op_node;
class binary_op_node;
class func_node;
class literal_node;
class id_node;

namespace impl {

template <template <typename> typename AddCv>
struct ast_visitor_cv {
  virtual void visit(AddCv<ast_node>&) {}
  virtual void visit(AddCv<unary_node>&) {}
  virtual void visit(AddCv<paren_node>&) {}
  virtual void visit(AddCv<unary_op_node>&) {}
  virtual void visit(AddCv<binary_op_node>&) {}
  virtual void visit(AddCv<func_node>&) {}
  virtual void visit(AddCv<literal_node>&) {}
  virtual void visit(AddCv<id_node>&) {}
  virtual void visit(AddCv<abs_node>&) {}
};

} // namespace impl

using ast_visitor = impl::ast_visitor_cv<util::identity>;
using const_ast_visitor = impl::ast_visitor_cv<std::add_const_t>;

} // namespace mparse