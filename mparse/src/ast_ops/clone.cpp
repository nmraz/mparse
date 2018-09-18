#include "clone.h"

#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/abs_node.h"
#include "mparse/ast/func_node.h"
#include "mparse/ast/id_node.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/paren_node.h"

namespace ast_ops {
namespace {

struct clone_visitor : mparse::const_ast_visitor {
  void visit(const mparse::paren_node& node) override;
  void visit(const mparse::abs_node& node) override;
  void visit(const mparse::unary_op_node& node) override;
  void visit(const mparse::binary_op_node& node) override;
  void visit(const mparse::func_node& node) override;
  void visit(const mparse::literal_node& node) override;
  void visit(const mparse::id_node& node) override;
};

}  // namespace


mparse::ast_node_ptr clone(const mparse::ast_node* node) {

}

}  // namespace ast_ops