#pragma once

#include <type_traits>

namespace mparse {

class ast_node;
class unary_node;
class paren_node;
class unary_op_node;
class binary_op_node;
class literal_node;

struct ast_visitor {
  virtual void visit(ast_node&) {}
  virtual void visit(unary_node&) {}
  virtual void visit(paren_node&) {}
  virtual void visit(unary_op_node&) {}
  virtual void visit(binary_op_node&) {}
  virtual void visit(literal_node&) {}
};

}  // namespace mparse