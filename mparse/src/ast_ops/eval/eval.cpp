#include "eval.h"

#include "ast_ops/eval/eval_error.h"
#include "mparse/ast/abs_node.h"
#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/func_node.h"
#include "mparse/ast/id_node.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include <cmath>
#include <sstream>
#include <system_error>
#include <vector>

using namespace std::literals;

namespace ast_ops {
namespace {

bool is_finite(number x) {
  return std::isfinite(x.real()) && std::isfinite(x.imag());
}

template <typename F>
number check_range(F func, const mparse::ast_node& node) {
  errno = 0;
  number res = func();
  if (errno || !is_finite(res)) {
    throw eval_error("Result too large", eval_errc::out_of_range, &node);
  }
  return res;
}

template <typename F>
number check_errno(F func) {
  errno = 0;
  number res = func();
  if (!errno && !is_finite(res)) {
    errno = ERANGE;
  }
  if (errno) {
    throw std::system_error(errno, std::generic_category());
  }
  return res;
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
  number result = 0;
};

eval_visitor::eval_visitor(const var_scope& vscope, const func_scope& fscope)
    : vscope(vscope), fscope(fscope) {}

void eval_visitor::visit(const mparse::unary_node& node) {
  node.child()->apply_visitor(*this);
}

void eval_visitor::visit(const mparse::abs_node& node) {
  result = check_range([&] { return std::abs(result); }, node);
}

void eval_visitor::visit(const mparse::unary_op_node& node) {
  if (node.type() == mparse::unary_op_type::neg) {
    result = check_range([&] { return -result; }, node);
  }
}

void eval_visitor::visit(const mparse::binary_op_node& node) {
  node.lhs()->apply_visitor(*this);
  number lhs_val = result;

  node.rhs()->apply_visitor(*this);
  number rhs_val = result;

  result = check_range(
      [&] {
        switch (node.type()) {
        case mparse::binary_op_type::add:
          return lhs_val + rhs_val;
        case mparse::binary_op_type::sub:
          return lhs_val - rhs_val;
        case mparse::binary_op_type::mult:
          return lhs_val * rhs_val;
        case mparse::binary_op_type::div:
          if (rhs_val == 0.0) {
            throw eval_error("Division by zero", eval_errc::div_by_zero, &node);
          }
          return lhs_val / rhs_val;
        case mparse::binary_op_type::pow:
          if (lhs_val == 0.0 && rhs_val.real() <= 0) {
            throw eval_error("Real part of exponent for zero must be positive",
                             eval_errc::bad_pow, &node);
          }
          return std::pow(lhs_val, rhs_val);
        default:
          return 0i; // deduce as complex
        }
      },
      node);
}

void eval_visitor::visit(const mparse::func_node& node) {
  auto* func = fscope.lookup(node.name());
  if (!func) {
    throw eval_error("Function '" + node.name() + "' not found.",
                     eval_errc::bad_func_call, &node);
  }

  std::vector<number> args;
  for (const auto& arg : node.args()) {
    arg->apply_visitor(*this);
    args.push_back(result);
  }

  try {
    result = check_errno([&] { return (*func)(std::move(args)); });
  } catch (...) {
    eval_error err("In function '" + node.name() + "'",
                   eval_errc::bad_func_call, &node);
    std::throw_with_nested(std::move(err));
  }
}

void eval_visitor::visit(const mparse::literal_node& node) {
  result = check_range([&] { return node.val(); }, node);
}

void eval_visitor::visit(const mparse::id_node& node) {
  if (auto val = vscope.lookup(node.name())) {
    result = check_range([&] { return *val; }, node);
  } else {
    throw eval_error("Unbound variable '" + node.name() + "'",
                     eval_errc::unbound_var, &node);
  }
}

} // namespace


number eval(const mparse::ast_node* node, const var_scope& vscope,
            const func_scope& fscope) {
  eval_visitor vis(vscope, fscope);
  node->apply_visitor(vis);
  return vis.result;
}

} // namespace ast_ops