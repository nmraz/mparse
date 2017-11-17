#include "eval.h"

#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/paren_node.h"
#include <cmath>

eval_error::eval_error(std::string_view what, std::vector<mparse::source_range> where)
  : std::runtime_error(what.data())
  , where_(std::move(where)) {
}

namespace {

struct eval_visitor : mparse::ast_visitor {
  void visit(mparse::paren_node& node) override;
  void visit(mparse::unary_op_node& node) override;
  void visit(mparse::binary_op_node& node) override;
  void visit(mparse::literal_node& node) override;

  double result = 0;
};

void eval_visitor::visit(mparse::paren_node& node) {
  node.child()->apply_visitor(*this);
}

void eval_visitor::visit(mparse::unary_op_node& node) {
  switch (node.type()) {
  case mparse::unary_op_type::plus:
    node.child()->apply_visitor(*this);
    break;
  case mparse::unary_op_type::neg:
    node.child()->apply_visitor(*this);
    result = -result;
    break;
  default:
    throw eval_error("Unknown unary operator", { mparse::source_range(node.op_loc()) });
  }
}

void eval_visitor::visit(mparse::binary_op_node& node) {
  node.lhs()->apply_visitor(*this);
  double lhs_val = result;

  node.rhs()->apply_visitor(*this);
  double rhs_val = result;

  switch (node.type()) {
  case mparse::binary_op_type::add:
    result = lhs_val + rhs_val;
    break;
  case mparse::binary_op_type::sub:
    result = lhs_val - rhs_val;
    break;
  case mparse::binary_op_type::mult:
    result = lhs_val * rhs_val;
    break;
  case mparse::binary_op_type::div:
    if (rhs_val == 0) {
      throw eval_error("Division by zero", { node.rhs()->source_loc() });
    }
    result = lhs_val / rhs_val;
    break;
  case mparse::binary_op_type::pow:
    if (lhs_val < 0 && rhs_val != static_cast<int>(rhs_val)) {
      throw eval_error(
        "Raising negative number to non-integer power",
        { node.lhs()->source_loc(), node.rhs()->source_loc() }
      );
    }
    if (lhs_val == 0 && rhs_val <= 0) {
      throw eval_error(
        "Raising zero to negative or zero power",
        { node.lhs()->source_loc(), node.rhs()->source_loc() }
      );
    }
    result = std::pow(lhs_val, rhs_val);
    break;
  default:
    throw eval_error("Unknown binary operator", { mparse::source_range(node.op_loc()) });
  }
}

void eval_visitor::visit(mparse::literal_node& node) {
  result = node.val();
}

}  // namespace

double eval(mparse::abstract_syntax_tree& ast) {
  eval_visitor vis;
  ast.root()->apply_visitor(vis);
  return vis.result;
}