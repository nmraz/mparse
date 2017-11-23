#include "ast_dump.h"

#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/id_node.h"
#include "mparse/ast/paren_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/literal_node.h"
#include "op_strings.h"
#include "util/auto_restore.h"
#include <iostream>
#include <string>

using namespace std::literals;

namespace ast_ops {
namespace {

std::string stringify_source_locs(const mparse::ast_node& node, const mparse::source_map* smap) {
  if (!smap) {
    return "";
  }

  std::string ret;
  for (const auto& loc : smap->find_locs(&node)) {
    ret += " <col:" + std::to_string(loc.from() + 1) + ", col:" + std::to_string(loc.to()) + ">";
  }

  return ret;
}

struct ast_dump_visitor : mparse::const_ast_visitor {
  ast_dump_visitor(std::string prefix, bool last_node, const mparse::source_map* smap, std::ostream& stream);

  void visit(const mparse::ast_node& node) override;
  void visit(const mparse::paren_node& node) override;
  void visit(const mparse::unary_op_node& node) override;
  void visit(const mparse::binary_op_node& node) override;
  void visit(const mparse::literal_node& node) override;
  void visit(const mparse::id_node& node) override;

  void dump_last_child(const mparse::ast_node& node);

  std::string prefix;
  bool last_node;

  const mparse::source_map* smap;
  std::ostream& stream;
};


ast_dump_visitor::ast_dump_visitor(std::string prefix, bool last_node, const mparse::source_map* smap, std::ostream& stream)
  : prefix(std::move(prefix))
  , last_node(last_node)
  , smap(smap)
  , stream(stream) {
}


void ast_dump_visitor::visit(const mparse::ast_node&) {
  stream << prefix;

  if (last_node) {
    stream << '`';  // last child - add corner
    prefix += ' ';  // account for extra character
  }

  stream << '-';
}

void ast_dump_visitor::visit(const mparse::paren_node& node) {
  stream << "paren" << stringify_source_locs(node, smap) << "\n";

  dump_last_child(*node.child());
}

void ast_dump_visitor::visit(const mparse::unary_op_node& node) {
  stream << "unary '" << stringify_unary_op(node.type()) << "'" << stringify_source_locs(node, smap) << "\n";

  dump_last_child(*node.child());
}

void ast_dump_visitor::visit(const mparse::binary_op_node& node) {
  stream << "binary '" << stringify_binary_op(node.type()) << "'" << stringify_source_locs(node, smap) << "\n";

  {
    util::auto_restore<std::string> save_prefix(prefix);
    prefix += " |";
    last_node = false;
    node.lhs()->apply_visitor(*this);
  }

  dump_last_child(*node.rhs());
}

void ast_dump_visitor::visit(const mparse::literal_node& node) {
  stream << "number '" << node.val() << "'" << stringify_source_locs(node, smap) << "\n";
}

void ast_dump_visitor::visit(const mparse::id_node& node) {
  stream << "variable '" << node.name() << "'" << stringify_source_locs(node, smap) << "\n";
}


void ast_dump_visitor::dump_last_child(const mparse::ast_node& node) {
  prefix += " ";
  last_node = true;
  node.apply_visitor(*this);
}

}  // namespace


void dump_ast(const mparse::ast_node* node, const mparse::source_map* smap, std::ostream& stream) {
  ast_dump_visitor vis("", true, smap, stream);
  node->apply_visitor(vis);
}

void dump_ast(const mparse::ast_node* node, const mparse::source_map* smap) {
  dump_ast(node, smap, std::cout);
}

}  // namespace ast_ops