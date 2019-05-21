#include "rewrite.h"

#include "mparse/ast.h"

namespace ast_ops::matching {
namespace {

struct child_apply_visitor : mparse::ast_visitor1<child_apply_visitor> {
  explicit child_apply_visitor(const basic_rewriter_func& func) : func(func) {}

  void operator()(mparse::unary_node& node);
  void operator()(mparse::binary_op_node& node);
  void operator()(mparse::func_node& node);

  const basic_rewriter_func& func;
};

void child_apply_visitor::operator()(mparse::unary_node& node) {
  auto child = node.ref_child(); // ref here for strong exception guarantee
  func(child);
  node.set_child(std::move(child));
}

void child_apply_visitor::operator()(mparse::binary_op_node& node) {
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

void child_apply_visitor::operator()(mparse::func_node& node) {
  for (auto& arg : node.args()) {
    func(arg);
  }
}

} // namespace


void apply_to_children(mparse::ast_node& node,
                       const basic_rewriter_func& func) {
  child_apply_visitor vis(func);
  mparse::apply_visitor(vis, node);
}

} // namespace ast_ops::matching