#include "ast_dump.h"

#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/paren_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/literal_node.h"
#include "op_strings.h"
#include <iostream>
#include <string>

using namespace std::literals;

namespace {

std::string stringify_source_locs(const mparse::ast_node& node, const mparse::source_map& smap) {
  std::string ret;
  for (const auto& loc : smap.find_locs(&node)) {
    ret += "<col:" + std::to_string(loc.from() + 1) + ", col:" + std::to_string(loc.to()) + "> ";
  }

  return ret;
}

struct ast_dump_visitor : mparse::const_ast_visitor {
  ast_dump_visitor(std::string prefx, bool last_node, const mparse::source_map& smap);

  void visit(const mparse::ast_node& node) override;
  void visit(const mparse::paren_node& node) override;
  void visit(const mparse::unary_op_node& node) override;
  void visit(const mparse::binary_op_node& node) override;
  void visit(const mparse::literal_node& node) override;

  void dump_last_child(const mparse::ast_node& node);

  std::string prefix;
  bool last_node;
  const mparse::source_map& smap;

  std::string result;
};


ast_dump_visitor::ast_dump_visitor(std::string prefix, bool last_node, const mparse::source_map& smap)
  : prefix(std::move(prefix))
  , last_node(last_node)
  , smap(smap) {
}


void ast_dump_visitor::visit(const mparse::ast_node&) {
  result += prefix;

  if (last_node) {
    result += '`';  // last child - add corner
    prefix += ' ';  // account for extra character
  }

  result += '-';
}

void ast_dump_visitor::visit(const mparse::paren_node& node) {
  result += "paren" + stringify_source_locs(node, smap) + "\n";

  dump_last_child(*node.child());
}

void ast_dump_visitor::visit(const mparse::unary_op_node& node) {
  result += "unary '"s + stringify_unary_op(node.type()) + "'" + stringify_source_locs(node, smap) + "\n";

  dump_last_child(*node.child());
}

void ast_dump_visitor::visit(const mparse::binary_op_node& node) {
  result += "binary '"s + stringify_binary_op(node.type()) + "'" + stringify_source_locs(node, smap) + "\n";

  ast_dump_visitor lhs_vis(prefix + " |", false, smap);
  node.lhs()->apply_visitor(lhs_vis);
  result += lhs_vis.result;

  dump_last_child(*node.rhs());
}

void ast_dump_visitor::visit(const mparse::literal_node& node) {
  result += "number '" + std::to_string(node.val()) + "'" + stringify_source_locs(node, smap) + "\n";
}


void ast_dump_visitor::dump_last_child(const mparse::ast_node& node) {
  prefix += " ";
  last_node = true;
  node.apply_visitor(*this);
}

}  // namespace


void dump_ast(const mparse::ast_node* node, const mparse::source_map& smap) {
  ast_dump_visitor vis("", false, smap);

  node->apply_visitor(vis);
  std::cout << vis.result;
}