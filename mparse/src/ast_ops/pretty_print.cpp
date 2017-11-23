#include "pretty_print.h"

#include "op_strings.h"
#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/paren_node.h"
#include "util/auto_restore.h"
#include <sstream>

using namespace std::literals;

namespace ast_ops {
namespace {

// all operators, in order of increasing precedence
enum class op_precedence {
  unknown,
  add,
  mult,
  unary,
  pow
};

op_precedence get_precedence(mparse::binary_op_type op) {
  switch (op) {
  case mparse::binary_op_type::add:
  case mparse::binary_op_type::sub:
    return op_precedence::add;

  case mparse::binary_op_type::mult:
  case mparse::binary_op_type::div:
    return op_precedence::mult;

  case mparse::binary_op_type::pow:
    return op_precedence::pow;
  }

  return op_precedence::unknown;  // force parenthesization
}

std::string parenthesize(std::string expr, op_precedence parent_precedence, op_precedence precedence) {
  if (static_cast<int>(precedence) < static_cast<int>(parent_precedence)) {
    return "(" + expr + ")";
  }
  return expr;
}


struct print_visitor : mparse::const_ast_visitor {
  void visit(const mparse::unary_node& node) override;
  void visit(const mparse::unary_op_node& node) override;
  void visit(const mparse::binary_op_node& node) override;
  void visit(const mparse::literal_node& node) override;

  std::string result;
  op_precedence parent_precedence = op_precedence::unknown;
};

void print_visitor::visit(const mparse::unary_node& node) {
  node.child()->apply_visitor(*this);
}

void print_visitor::visit(const mparse::unary_op_node& node) {
  result = parenthesize(stringify_unary_op(node.type()) + result, parent_precedence, op_precedence::unary);
}

void print_visitor::visit(const mparse::binary_op_node& node) {
  mparse::binary_op_type op = node.type();
  op_precedence prec = get_precedence(op);

  std::string lhs_str, rhs_str;
  {
    util::auto_restore<op_precedence> save_parent_precedence(parent_precedence);
    parent_precedence = prec;

    node.lhs()->apply_visitor(*this);
    lhs_str = std::move(result);

    node.rhs()->apply_visitor(*this);
    rhs_str = std::move(result);
  }

  result = parenthesize(lhs_str + " " + stringify_binary_op(op) + " " + rhs_str, parent_precedence, prec);
}

void print_visitor::visit(const mparse::literal_node& node) {
  std::ostringstream stream;
  stream << node.val();
  result = stream.str();
}

}  // namespace


std::string pretty_print(const mparse::ast_node* node) {
  print_visitor vis;
  node->apply_visitor(vis);
  return vis.result;
}

}  // namespace ast_ops