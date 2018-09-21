#include "rewrite.h"

#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/func_node.h"
#include "mparse/ast/operator_nodes.h"

namespace ast_ops::matching {
namespace {

struct apply_rec_visitor : mparse::ast_visitor {
  apply_rec_visitor(const rewriter_func& func) : func(func) {}

  void visit(mparse::unary_node& node) override;
  void visit(mparse::binary_op_node& node) override;
  void visit(mparse::func_node& node) override;

  const rewriter_func& func;
  bool result = false;
};

void apply_rec_visitor::visit(mparse::unary_node& node) {
  auto child = node.ref_child(); // ref here for strong exception guarantee
  child->apply_visitor(*this);
  result |= func(child); // no short-circuiting
  node.set_child(std::move(child));
}

void apply_rec_visitor::visit(mparse::binary_op_node& node) {
  {
    auto lhs = node.ref_lhs(); // ref here for strong exception guarantee
    lhs->apply_visitor(*this);
    result |= func(lhs); // no short-circuiting
    node.set_lhs(std::move(lhs));
  }

  {
    auto rhs = node.ref_rhs(); // ref here for strong exception guarantee
    rhs->apply_visitor(*this);
    result |= func(rhs); // no short-circuiting
    node.set_rhs(std::move(rhs));
  }
}

void apply_rec_visitor::visit(mparse::func_node& node) {
  for (auto& arg : node.args()) {
    arg->apply_visitor(*this);
    result |= func(arg);
  }
}


} // namespace

bool apply_recursively(mparse::ast_node_ptr& node, rewriter_func func) {
  apply_rec_visitor vis(func);
  node->apply_visitor(vis);
  return func(node) | vis.result;
}

} // namespace ast_ops::matching