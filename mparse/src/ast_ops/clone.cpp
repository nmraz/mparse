#include "clone.h"

#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/abs_node.h"
#include "mparse/ast/func_node.h"
#include "mparse/ast/id_node.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/paren_node.h"
#include "mparse/ast/unary_node.h"

namespace ast_ops {
namespace {

struct clone_visitor : mparse::const_ast_visitor {
  void visit(const mparse::unary_node& node) override;
  void visit(const mparse::paren_node& node) override;
  void visit(const mparse::abs_node& node) override;
  void visit(const mparse::unary_op_node& node) override;
  void visit(const mparse::binary_op_node& node) override;
  void visit(const mparse::func_node& node) override;
  void visit(const mparse::literal_node& node) override;
  void visit(const mparse::id_node& node) override;

  mparse::ast_node_ptr cloned = nullptr;
};


void clone_visitor::visit(const mparse::unary_node& node) {
  node.child()->apply_visitor(*this);
}

void clone_visitor::visit(const mparse::paren_node&) {
  cloned = mparse::make_ast_node<mparse::paren_node>(std::move(cloned));
}

void clone_visitor::visit(const mparse::abs_node&) {
  cloned = mparse::make_ast_node<mparse::abs_node>(std::move(cloned));
}

void clone_visitor::visit(const mparse::unary_op_node& node) {
  cloned = mparse::make_ast_node<mparse::unary_op_node>(node.type(), std::move(cloned));
}

void clone_visitor::visit(const mparse::binary_op_node& node) {
  node.lhs()->apply_visitor(*this);
  auto cloned_lhs = std::move(cloned);

  node.rhs()->apply_visitor(*this);
  auto cloned_rhs = std::move(cloned);

  cloned = mparse::make_ast_node<mparse::binary_op_node>(node.type(),
    std::move(cloned_lhs), std::move(cloned_rhs));
}

void clone_visitor::visit(const mparse::func_node& node) {
  mparse::func_node::arg_list cloned_args;
  for (const auto& arg : node.args()) {
    arg->apply_visitor(*this);
    cloned_args.push_back(std::move(cloned));
  }

  cloned = mparse::make_ast_node<mparse::func_node>(node.name(),
    std::move(cloned_args));
}

void clone_visitor::visit(const mparse::literal_node& node) {
  cloned = mparse::make_ast_node<mparse::literal_node>(node.val());
}

void clone_visitor::visit(const mparse::id_node& node) {
  cloned = mparse::make_ast_node<mparse::id_node>(node.name());
}

}  // namespace


mparse::ast_node_ptr clone(const mparse::ast_node* node) {
  clone_visitor vis;
  node->apply_visitor(vis);
  return vis.cloned;
}

}  // namespace ast_ops