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
  ast_dump_visitor(std::string prefx, bool last_node);

  void visit(mparse::ast_node& node) override;
  void visit(mparse::paren_node& node) override;
  void visit(mparse::unary_op_node& node) override;
  void visit(mparse::binary_op_node& node) override;
  void visit(mparse::literal_node& node) override;

  void dump_last_child(mparse::ast_node& node);

  std::string prefix;
  bool last_node;

  std::string result;
};


std::string stringify_node_loc(const mparse::ast_node& node) {
  auto loc = node.source_loc();
  return "<col:" + std::to_string(loc.from() + 1) + ", col:" + std::to_string(loc.to()) + ">";
}


ast_dump_visitor::ast_dump_visitor(std::string prefix, bool last_node)
  : prefix(std::move(prefix))
  , last_node(last_node) {
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

  dump_last_child(*node.child());
}

void ast_dump_visitor::visit(mparse::unary_op_node& node) {
  result += "unary " + stringify_node_loc(node) + " '" + stringify_unary_op(node.type()) + "'\n";

  dump_last_child(*node.child());
}

void ast_dump_visitor::visit(mparse::binary_op_node& node) {
  result += "binary " + stringify_node_loc(node) + " '" + stringify_binary_op(node.type()) + "'\n";

  ast_dump_visitor lhs_vis(prefix + " |", false);
  node.lhs()->apply_visitor(lhs_vis);
  result += lhs_vis.result;

  dump_last_child(*node.rhs());
}

void ast_dump_visitor::visit(mparse::literal_node& node) {
  result += "number " + stringify_node_loc(node) + " '" + std::to_string(node.val()) + "'\n";
}


void ast_dump_visitor::dump_last_child(mparse::ast_node& node) {
  prefix += " ";
  last_node = true;
  node.apply_visitor(*this);
}

}  // namespace


void dump_ast(mparse::abstract_syntax_tree& ast) {
  ast_dump_visitor vis("", false);

  ast.root()->apply_visitor(vis);
  std::cout << vis.result;
}