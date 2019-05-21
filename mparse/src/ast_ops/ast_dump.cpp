#include "ast_dump.h"

#include "mparse/ast.h"
#include "op_strings.h"
#include "util/auto_restore.h"
#include <iostream>
#include <string>

using namespace std::literals;

namespace ast_ops {
namespace {

std::string stringify_source_locs(const mparse::ast_node& node,
                                  const mparse::source_map* smap) {
  if (!smap) {
    return "";
  }

  std::string ret;
  for (const auto& loc : smap->find_locs(&node)) {
    ret += " <" + std::to_string(loc.from() + 1) + "," +
           std::to_string(loc.to()) + ">";
  }

  return ret;
}

struct ast_dump_visitor {
  ast_dump_visitor(std::string prefix, bool last_node,
                   const mparse::source_map* smap, std::ostream& stream);

  template <typename T>
  void operator()(T&&) {}

  void operator()(const mparse::ast_node& node);
  void operator()(const mparse::paren_node& node);
  void operator()(const mparse::abs_node& node);
  void operator()(const mparse::unary_op_node& node);
  void operator()(const mparse::binary_op_node& node);
  void operator()(const mparse::func_node& node);
  void operator()(const mparse::literal_node& node);
  void operator()(const mparse::id_node& node);

  void dump_child(const mparse::ast_node& node);
  void dump_last_child(const mparse::ast_node& node);

  std::string prefix;
  bool last_node;

  const mparse::source_map* smap;
  std::ostream& stream;
};


ast_dump_visitor::ast_dump_visitor(std::string prefix, bool last_node,
                                   const mparse::source_map* smap,
                                   std::ostream& stream)
    : prefix(std::move(prefix)),
      last_node(last_node),
      smap(smap),
      stream(stream) {}


void ast_dump_visitor::operator()(const mparse::ast_node&) {
  stream << prefix;

  if (last_node) {
    stream << '`'; // last child - add corner
    prefix += ' '; // account for extra character
  }

  stream << '-';
}

void ast_dump_visitor::operator()(const mparse::paren_node& node) {
  stream << "paren" << stringify_source_locs(node, smap) << "\n";

  dump_last_child(*node.child());
}

void ast_dump_visitor::operator()(const mparse::abs_node& node) {
  stream << "abs" << stringify_source_locs(node, smap) << "\n";

  dump_last_child(*node.child());
}

void ast_dump_visitor::operator()(const mparse::unary_op_node& node) {
  stream << "unary '" << stringify_unary_op(node.type()) << "'"
         << stringify_source_locs(node, smap) << "\n";

  dump_last_child(*node.child());
}

void ast_dump_visitor::operator()(const mparse::binary_op_node& node) {
  stream << "binary '" << stringify_binary_op(node.type()) << "'"
         << stringify_source_locs(node, smap) << "\n";

  dump_child(*node.lhs());
  dump_last_child(*node.rhs());
}

void ast_dump_visitor::operator()(const mparse::func_node& node) {
  stream << "func '" << node.name() << "'" << stringify_source_locs(node, smap)
         << "\n";

  for (const auto& arg : node.args()) {
    if (arg == node.args().back()) {
      dump_last_child(*arg);
    } else {
      dump_child(*arg);
    }
  }
}

void ast_dump_visitor::operator()(const mparse::literal_node& node) {
  stream << "number '" << node.val() << "'" << stringify_source_locs(node, smap)
         << "\n";
}

void ast_dump_visitor::operator()(const mparse::id_node& node) {
  stream << "variable '" << node.name() << "'"
         << stringify_source_locs(node, smap) << "\n";
}


void ast_dump_visitor::dump_child(const mparse::ast_node& node) {
  util::auto_restore save_prefix(prefix);
  prefix += " |";
  last_node = false;
  mparse::apply_visitor(*this, node);
}

void ast_dump_visitor::dump_last_child(const mparse::ast_node& node) {
  prefix += " ";
  last_node = true;
  mparse::apply_visitor(*this, node);
}

} // namespace


void dump_ast(const mparse::ast_node& node, const mparse::source_map* smap,
              std::ostream& stream) {
  ast_dump_visitor vis("", false, smap, stream);
  mparse::apply_visitor(vis, node);
}

void dump_ast(const mparse::ast_node& node, const mparse::source_map* smap) {
  dump_ast(node, smap, std::cout);
}

} // namespace ast_ops