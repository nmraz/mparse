#include "clone.h"

#include "mparse/ast.h"

namespace ast_ops {
namespace {

struct clone_visitor : mparse::const_ast_visitor<clone_visitor> {
  void operator()(const mparse::unary_node& node);
  void operator()(const mparse::paren_node& node);
  void operator()(const mparse::abs_node& node);
  void operator()(const mparse::unary_op_node& node);
  void operator()(const mparse::binary_op_node& node);
  void operator()(const mparse::func_node& node);
  void operator()(const mparse::literal_node& node);
  void operator()(const mparse::id_node& node);

  mparse::ast_node_ptr cloned = nullptr;
};


void clone_visitor::operator()(const mparse::unary_node& node) {
  mparse::apply_visitor(*this, *node.child());
}

void clone_visitor::operator()(const mparse::paren_node&) {
  cloned = mparse::make_ast_node<mparse::paren_node>(std::move(cloned));
}

void clone_visitor::operator()(const mparse::abs_node&) {
  cloned = mparse::make_ast_node<mparse::abs_node>(std::move(cloned));
}

void clone_visitor::operator()(const mparse::unary_op_node& node) {
  cloned = mparse::make_ast_node<mparse::unary_op_node>(node.type(),
                                                        std::move(cloned));
}

void clone_visitor::operator()(const mparse::binary_op_node& node) {
  mparse::apply_visitor(*this, *node.lhs());
  auto cloned_lhs = std::move(cloned);

  mparse::apply_visitor(*this, *node.rhs());
  auto cloned_rhs = std::move(cloned);

  cloned = mparse::make_ast_node<mparse::binary_op_node>(
      node.type(), std::move(cloned_lhs), std::move(cloned_rhs));
}

void clone_visitor::operator()(const mparse::func_node& node) {
  mparse::func_node::arg_list cloned_args;
  for (const auto& arg : node.args()) {
    mparse::apply_visitor(*this, *arg);
    cloned_args.push_back(std::move(cloned));
  }

  cloned = mparse::make_ast_node<mparse::func_node>(node.name(),
                                                    std::move(cloned_args));
}

void clone_visitor::operator()(const mparse::literal_node& node) {
  cloned = mparse::make_ast_node<mparse::literal_node>(node.val());
}

void clone_visitor::operator()(const mparse::id_node& node) {
  cloned = mparse::make_ast_node<mparse::id_node>(node.name());
}

} // namespace


mparse::ast_node_ptr clone(const mparse::ast_node& node) {
  clone_visitor vis;
  mparse::apply_visitor(vis, node);
  return vis.cloned;
}

} // namespace ast_ops