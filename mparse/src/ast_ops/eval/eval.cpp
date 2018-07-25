#include "eval.h"

#include "ast_ops/eval/exceptions.h"
#include "mparse/ast/abs_node.h"
#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/func_node.h"
#include "mparse/ast/id_node.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include <cmath>
#include <sstream>
#include <vector>

namespace ast_ops {
namespace {

[[noreturn]] void throw_arity_error(int expected, int provided) {
  std::ostringstream msg;
  msg << "wrong number of arguments (" << expected << " expected, " << provided << " provided)";
  throw arity_error(msg.str(), expected, provided);
}

template<typename F>
void call_with_err_wrap(F&& func, const mparse::func_node& node) {
  try {
    std::forward<F>(func)();
  } catch (...) {
    eval_error err("Error in function '" + node.name() + "'", eval_errc::bad_func_call, &node);
    std::throw_with_nested(std::move(err));
  }
}


struct eval_visitor : mparse::const_ast_visitor {
  eval_visitor(const var_scope& vscope, const func_scope& fscope);

  void visit(const mparse::unary_node& node) override;
  void visit(const mparse::abs_node& node) override;
  void visit(const mparse::unary_op_node& node) override;
  void visit(const mparse::binary_op_node& node) override;
  void visit(const mparse::func_node& node) override;
  void visit(const mparse::literal_node& node) override;
  void visit(const mparse::id_node& node) override;

  const var_scope& vscope;
  const func_scope& fscope;
  double result = 0;
};

eval_visitor::eval_visitor(const var_scope& vscope, const func_scope& fscope)
  : vscope(vscope)
  , fscope(fscope) {
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

void eval_visitor::visit(const mparse::func_node& node) {
  auto* ent = fscope.lookup(node.name());
  if (!ent) {
    throw eval_error("Function '" + node.name() + "' not found.", eval_errc::bad_func_call, &node);
  }

  call_with_err_wrap([&] {
    if (ent->arity && ent->arity != node.args().size()) {
      throw_arity_error(*ent->arity, static_cast<int>(node.args().size()));
    }
  }, node);

  std::vector<double> args;
  for (const auto& arg : node.args()) {
    arg->apply_visitor(*this);
    args.push_back(result);
  }

  call_with_err_wrap([&] {
    ent->func(std::move(args));
  }, node);
}

void eval_visitor::visit(const mparse::literal_node& node) {
  result = node.val();
}

void eval_visitor::visit(const mparse::id_node& node) {
  if (auto val = vscope.lookup(node.name())) {
    result = *val;
  } else {
    throw eval_error("Unbound variable '" + node.name() + "'", eval_errc::unbound_var, &node);
  }
}

}  // namespace


double eval(const mparse::ast_node* node, const var_scope& vscope, const func_scope& fscope) {
  eval_visitor vis(vscope, fscope);
  node->apply_visitor(vis);
  return vis.result;
}

}  // namespac ast_ops