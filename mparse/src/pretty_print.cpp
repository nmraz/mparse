#include "pretty_print.h"

#include "op_strings.h"
#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/paren_node.h"
#include <sstream>

using namespace std::literals;

namespace {

struct print_visitor : mparse::const_ast_visitor {
  void visit(const mparse::unary_node& node) override;
  void visit(const mparse::unary_op_node& node) override;
  void visit(const mparse::binary_op_node& node) override;
  void visit(const mparse::literal_node& node) override;

  std::string result;
};

void print_visitor::visit(const mparse::unary_node& node) {
  node.child()->apply_visitor(*this);
}

void print_visitor::visit(const mparse::unary_op_node& node) {
  result = "("s + stringify_unary_op(node.type()) + result + ")";
}

void print_visitor::visit(const mparse::binary_op_node& node) {
  node.lhs()->apply_visitor(*this);
  std::string lhs_str = std::move(result);

  node.rhs()->apply_visitor(*this);
  std::string rhs_str = std::move(result);

  result = "(" + lhs_str + " " + stringify_binary_op(node.type()) + " " + rhs_str + ")";
}

void print_visitor::visit(const mparse::literal_node& node) {
  std::ostringstream stream;
  stream << node.val();
  result = stream.str();
}

}  // namespace

std::string pretty_print(const mparse::abstract_syntax_tree& ast) {
  print_visitor vis;
  ast.root()->apply_visitor(vis);
  return vis.result;
}