#include "pretty_print.h"

#include "op_strings.h"
#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/id_node.h"
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

class auto_parenthesizer {
public:
  auto_parenthesizer(std::string& expr, op_precedence parent_precedence, op_precedence precedence);
  ~auto_parenthesizer();

private:
  std::string& expr_;
  bool parenthesize_;
};

auto_parenthesizer::auto_parenthesizer(std::string& expr, op_precedence parent_precedence, op_precedence precedence)
  : expr_(expr)
  , parenthesize_(static_cast<int>(precedence) < static_cast<int>(parent_precedence)) {
  if (parenthesize_) {
    expr_ += "(";
  }
}

auto_parenthesizer::~auto_parenthesizer() {
  if (parenthesize_) {
    expr_ += ")";
  }
}


struct print_visitor : mparse::const_ast_visitor {
  void visit(const mparse::paren_node& node) override;
  void visit(const mparse::unary_op_node& node) override;
  void visit(const mparse::binary_op_node& node) override;
  void visit(const mparse::literal_node& node) override;
  void visit(const mparse::id_node& node) override;

  std::string result;
  op_precedence parent_precedence = op_precedence::unknown;
};

void print_visitor::visit(const mparse::paren_node& node) {
  node.child()->apply_visitor(*this);
}

void print_visitor::visit(const mparse::unary_op_node& node) {
  print_visitor child_vis;
  child_vis.parent_precedence = op_precedence::unary;
  node.child()->apply_visitor(child_vis);

  auto_parenthesizer paren(result, parent_precedence, op_precedence::unary);
  result += stringify_unary_op(node.type()) + child_vis.result;
}

void print_visitor::visit(const mparse::binary_op_node& node) {
  mparse::binary_op_type op = node.type();
  op_precedence prec = get_precedence(op);

  auto_parenthesizer paren(result, parent_precedence, prec);

  util::auto_restore<op_precedence> restore_precedence(parent_precedence);
  parent_precedence = prec;

  node.lhs()->apply_visitor(*this);
  result += " "s + stringify_binary_op(op) + " ";
  node.rhs()->apply_visitor(*this);
}

void print_visitor::visit(const mparse::literal_node& node) {
  std::ostringstream stream;
  stream << node.val();
  result += stream.str();
}

void print_visitor::visit(const mparse::id_node& node) {
  result += node.name();
}

}  // namespace


std::string pretty_print(const mparse::ast_node* node) {
  print_visitor vis;
  node->apply_visitor(vis);
  return vis.result;
}

}  // namespace ast_ops