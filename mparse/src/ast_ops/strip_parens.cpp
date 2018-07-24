#include "strip_parens.h"

#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/cast.h"
#include "mparse/ast/func_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/paren_node.h"
#include "mparse/ast/unary_node.h"

namespace ast_ops {
namespace {

mparse::ast_node_ptr strip_paren(mparse::ast_node_ptr node) {
  if (auto* paren_node = mparse::ast_node_cast<mparse::paren_node>(node.get())) {
    return paren_node->take_child();
  }
  return node;
}


struct strip_paren_visitor : mparse::ast_visitor {
  void visit(mparse::unary_node& node) override;
  void visit(mparse::binary_op_node& node) override;
  void visit(mparse::func_node& node) override;
};

void strip_paren_visitor::visit(mparse::unary_node& node) {
  node.child()->apply_visitor(*this);
  node.set_child(strip_paren(node.take_child()));
}

void strip_paren_visitor::visit(mparse::binary_op_node& node) {
  node.lhs()->apply_visitor(*this);
  node.set_lhs(strip_paren(node.take_lhs()));

  node.rhs()->apply_visitor(*this);
  node.set_rhs(strip_paren(node.take_rhs()));
}

void strip_paren_visitor::visit(mparse::func_node& node) {
  for (auto& arg : node.args()) {
    arg->apply_visitor(*this);
    arg = strip_paren(arg);
  }
}

}  // namespace


void strip_parens(mparse::ast_node_ptr& node) {
  strip_paren_visitor vis;
  node->apply_visitor(vis);
  node = strip_paren(std::move(node));
}

}  // namespace ast_ops