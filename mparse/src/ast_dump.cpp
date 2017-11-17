#include "ast_dump.h"

#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/paren_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/literal_node.h"
#include "op_strings.h"
#include <iostream>
#include <string>

namespace {

struct ast_dump_visitor : mparse::ast_visitor {
  void visit(mparse::ast_node& node) override;
  void visit(mparse::paren_node& node) override;
  void visit(mparse::unary_op_node& node) override;
  void visit(mparse::binary_op_node& node) override;
  void visit(mparse::literal_node& node) override;

  std::string prefix;
  bool last_node;

  std::string result;
};


std::string stringify_node_loc(const mparse::ast_node& node) {
  auto loc = node.source_loc();
  return "<col:" + std::to_string(loc.from() + 1) + ", col:" + std::to_string(loc.to()) + ">";
}


ast_dump_visitor get_child_visitor(const std::string& prefix, bool last_node) {
  ast_dump_visitor vis;
  vis.last_node = last_node;
  vis.prefix = prefix + ' ';  // indent children
  if (!last_node) {
    vis.prefix += '|'; // add connecting line for children
  }

  return vis;
}


void ast_dump_visitor::visit(mparse::ast_node&) {
  result += prefix;

  if (last_node) {
    result += '`';  // last child - add corner
    prefix += ' ';  // account for extra character
  }

  result += '-';
}

void ast_dump_visitor::visit(mparse::paren_node& node) {
  result += "paren " + stringify_node_loc(node) + '\n';

  ast_dump_visitor child_vis = get_child_visitor(prefix, true);
  node.child()->apply_visitor(child_vis);
  result += child_vis.result;
}

void ast_dump_visitor::visit(mparse::unary_op_node& node) {
  result += "unary " + stringify_node_loc(node) + " '" + stringify_unary_op(node.type()) + "'\n";

  ast_dump_visitor child_vis = get_child_visitor(prefix, true);
  node.child()->apply_visitor(child_vis);
  result += child_vis.result;
}

void ast_dump_visitor::visit(mparse::binary_op_node& node) {
  result += "binary " + stringify_node_loc(node) + " '" + stringify_binary_op(node.type()) + "'\n";

  ast_dump_visitor lhs_vis = get_child_visitor(prefix, false);
  node.lhs()->apply_visitor(lhs_vis);
  result += lhs_vis.result;

  ast_dump_visitor rhs_vis = get_child_visitor(prefix, true);
  node.rhs()->apply_visitor(rhs_vis);
  result += rhs_vis.result;
}

void ast_dump_visitor::visit(mparse::literal_node& node) {
  result += "number " + stringify_node_loc(node) + " '" + std::to_string(node.val()) + "'\n";
}

}  // namespace


void dump_ast(mparse::abstract_syntax_tree& ast) {
  ast_dump_visitor vis;
  vis.last_node = false;

  ast.root()->apply_visitor(vis);
  std::cout << vis.result;
}