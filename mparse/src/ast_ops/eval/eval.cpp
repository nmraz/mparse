#include "eval.h"

#include "mparse/ast/abs_node.h"
#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/id_node.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include <cmath>

namespace ast_ops {
namespace {

struct eval_visitor : mparse::const_ast_visitor {
  eval_visitor(const var_scope& scope);

  void visit(const mparse::unary_node& node) override;
  void visit(const mparse::abs_node& node) override;
  void visit(const mparse::unary_op_node& node) override;
  void visit(const mparse::binary_op_node& node) override;
  void visit(const mparse::literal_node& node) override;
  void visit(const mparse::id_node& node) override;

  const var_scope& scope;
  double result = 0;
};

eval_visitor::eval_visitor(const var_scope& scope)
  : scope(scope) {
}

void eval_visitor::visit(const mparse::unary_node& node) {
  node.child()->apply_visitor(*this);
}

void eval_visitor::visit(const mparse::abs_node&) {
  result = std::abs(result);
}

void eval_visitor::visit(const mparse::unary_op_node& node) {
  if (node.type() == mparse::unary_op_type::neg) {
    result = -result;
  }
}

void eval_visitor::visit(const mparse::binary_op_node& node) {
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
      throw eval_error("Division by zero", eval_errc::div_by_zero, &node);
    }
    result = lhs_val / rhs_val;
    break;
  case mparse::binary_op_type::pow:
    if (lhs_val < 0 && rhs_val != std::round(rhs_val)) {
      throw eval_error(
        "Raising negative number to non-integer power",
        eval_errc::bad_pow,
        &node
      );
    }
    if (lhs_val == 0 && rhs_val <= 0) {
      throw eval_error(
        "Raising zero to negative or zero power",
        eval_errc::bad_pow,
        &node
      );
    }
    result = std::pow(lhs_val, rhs_val);
    break;
  default:
    break;
  }
}

void eval_visitor::visit(const mparse::literal_node& node) {
  result = node.val();
}

void eval_visitor::visit(const mparse::id_node& node) {
  if (auto val = scope.lookup(node.name())) {
    result = *val;
  } else {
    throw eval_error("Unbound variable '" + node.name() + "'", eval_errc::unbound_var, &node);
  }
}

}  // namespace


eval_error::eval_error(std::string_view what, eval_errc code, const mparse::ast_node* node)
  : std::runtime_error(what.data())
  , code_(code)
  , node_(node) {
}

double eval(const mparse::ast_node* node, const var_scope& scope) {
  eval_visitor vis(scope);
  node->apply_visitor(vis);
  return vis.result;
}

}  // namespac ast_ops