#include "rewrite.h"

#include "mparse/ast.h"
#include "mparse/ast/ast_visitor.h"

namespace ast_ops::matching {
namespace {

struct child_apply_visitor : mparse::ast_visitor {
  explicit child_apply_visitor(const basic_rewriter_func& func) : func(func) {}

  void visit(mparse::unary_node& node) override;
  void visit(mparse::binary_op_node& node) override;
  void visit(mparse::func_node& node) override;

  const basic_rewriter_func& func;
};

void child_apply_visitor::visit(mparse::unary_node& node) {
  auto child = node.ref_child(); // ref here for strong exception guarantee
  func(child);
  node.set_child(std::move(child));
}

void child_apply_visitor::visit(mparse::binary_op_node& node) {
  {
    auto lhs = node.ref_lhs(); // ref here for strong exception guarantee
    func(lhs);
    node.set_lhs(std::move(lhs));
  }

  {
    auto rhs = node.ref_rhs(); // ref here for strong exception guarantee
    func(rhs);
    node.set_rhs(std::move(rhs));
  }
}

void child_apply_visitor::visit(mparse::func_node& node) {
  for (auto& arg : node.args()) {
    func(arg);
  }
}

} // namespace


void apply_to_children(mparse::ast_node& node,
                       const basic_rewriter_func& func) {
  child_apply_visitor vis(func);
  node.apply_visitor(vis);
}

} // namespace ast_ops::matching