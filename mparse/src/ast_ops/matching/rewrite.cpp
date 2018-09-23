#include "rewrite.h"

#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/func_node.h"
#include "mparse/ast/operator_nodes.h"

namespace ast_ops::matching {
namespace {

struct child_apply_visitor : mparse::ast_visitor {
  child_apply_visitor(const rewriter_func& func) : func(func) {}

  void visit(mparse::unary_node& node) override;
  void visit(mparse::binary_op_node& node) override;
  void visit(mparse::func_node& node) override;

  const rewriter_func& func;
  bool result = false;
};

void child_apply_visitor::visit(mparse::unary_node& node) {
  auto child = node.ref_child(); // ref here for strong exception guarantee
  result |= func(child);         // no short-circuiting
  node.set_child(std::move(child));
}

void child_apply_visitor::visit(mparse::binary_op_node& node) {
  {
    auto lhs = node.ref_lhs(); // ref here for strong exception guarantee
    result |= func(lhs);       // no short-circuiting
    node.set_lhs(std::move(lhs));
  }

  {
    auto rhs = node.ref_rhs(); // ref here for strong exception guarantee
    result |= func(rhs);       // no short-circuiting
    node.set_rhs(std::move(rhs));
  }
}

void child_apply_visitor::visit(mparse::func_node& node) {
  for (auto& arg : node.args()) {
    result |= func(arg);
  }
}

} // namespace


bool apply_to_children(mparse::ast_node_ptr& node, const rewriter_func& func) {
  child_apply_visitor vis(func);
  node->apply_visitor(vis);
  return vis.result;
}

} // namespace ast_ops::matching