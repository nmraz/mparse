#include "eval.h"

#include "ast_ops/eval/eval_error.h"
#include "mparse/ast.h"
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


struct eval_visitor : mparse::const_ast_visitor1<eval_visitor> {
  eval_visitor(const var_scope& vscope, const func_scope& fscope);

  void operator()(const mparse::unary_node& node);
  void operator()(const mparse::abs_node& node);
  void operator()(const mparse::unary_op_node& node);
  void operator()(const mparse::binary_op_node& node);
  void operator()(const mparse::func_node& node);
  void operator()(const mparse::literal_node& node);
  void operator()(const mparse::id_node& node);

  const var_scope& vscope;
  const func_scope& fscope;
  number result = 0;
};

eval_visitor::eval_visitor(const var_scope& vscope, const func_scope& fscope)
    : vscope(vscope), fscope(fscope) {}

void eval_visitor::operator()(const mparse::unary_node& node) {
  mparse::apply_visitor(*this, *node.child());
}

void eval_visitor::operator()(const mparse::abs_node& node) {
  result = check_range([&] { return std::abs(result); }, node);
}

void eval_visitor::operator()(const mparse::unary_op_node& node) {
  if (node.type() == mparse::unary_op_type::neg) {
    result = check_range([&] { return -result; }, node);
  }
}

void eval_visitor::operator()(const mparse::binary_op_node& node) {
  mparse::apply_visitor(*this, *node.lhs());
  number lhs_val = result;

  mparse::apply_visitor(*this, *node.rhs());
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
          if (lhs_val == 0.0) {
            if (rhs_val.imag()) {
              throw eval_error("Raising zero to complex power",
                               eval_errc::bad_pow, &node);
            }
            if (rhs_val.real() < 0) {
              throw eval_error("Raising zero to negative power",
                               eval_errc::bad_pow, &node);
            }
          }

          return std::pow(lhs_val, rhs_val);
        default:
          return 0i; // deduce as complex
        }
      },
      node);
}

void eval_visitor::operator()(const mparse::func_node& node) {
  auto* func = fscope.lookup(node.name());
  if (!func) {
    throw eval_error("Function '" + node.name() + "' not found",
                     eval_errc::bad_func_call, &node);
  }

  std::vector<number> args;
  for (const auto& arg : node.args()) {
    mparse::apply_visitor(*this, *arg);
    args.push_back(result);
  }

  try {
    result = check_errno([&] { return (*func)(args); });
  } catch (...) {
    eval_error err("In function '" + node.name() + "'",
                   eval_errc::bad_func_call, &node);
    std::throw_with_nested(std::move(err));
  }
}

void eval_visitor::operator()(const mparse::literal_node& node) {
  result = check_range([&] { return node.val(); }, node);
}

void eval_visitor::operator()(const mparse::id_node& node) {
  if (auto val = vscope.lookup(node.name())) {
    result = check_range([&] { return *val; }, node);
  } else {
    throw eval_error("Unbound variable '" + node.name() + "'",
                     eval_errc::unbound_var, &node);
  }
}

} // namespace


number eval(const mparse::ast_node& node, const var_scope& vscope,
            const func_scope& fscope) {
  eval_visitor vis(vscope, fscope);
  mparse::apply_visitor(vis, node);
  return vis.result;
}

} // namespace ast_ops