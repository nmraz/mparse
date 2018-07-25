#include "builtins.h"

#include "ast_ops/eval/eval_error.h"
#include <numeric>

namespace builtins {

double mod(double a, double b) {
  if (b == 0) {
    throw ast_ops::func_arg_error("mod by zero", { 1 });
  }
  return std::fmod(a, b);
}

double log(double base, double val) {
  if (val <= 0) {
    throw ast_ops::func_arg_error("argument out of domain", { 1 });
  }
  if (base <= 0 || base == 1) {
    throw ast_ops::func_arg_error("base out of domain", { 0 });
  }
  return std::log(val) / std::log(base);
}

double nroot(double n, double val) {
  if (val < 0 && std::fmod(n, 2) != 1) {
    throw ast_ops::func_arg_error("taking non-odd nth-root of negative number", { 0, 1 });
  }
  if (n == 0) {
    throw ast_ops::func_arg_error("taking zero-th root of number", { 0 });
  }
  if (val == 0 && n < 0) {
    throw ast_ops::func_arg_error("taking negative root of zero", { 0, 1 });
  }

  if (val < 0) {
    return -std::pow(-val, 1 / n);
  }
  return std::pow(val, 1 / n);
}

double min(std::vector<double> vals) {
  if (vals.empty()) {
    throw ast_ops::arity_error("at least one argument is required", 1, 0);
  }
  return *std::min_element(vals.begin(), vals.end());
}

double max(std::vector<double> vals) {
  if (vals.empty()) {
    throw ast_ops::arity_error("at least one argument is required", 1, 0);
  }
  return *std::max_element(vals.begin(), vals.end());
}

double avg(std::vector<double> vals) {
  if (vals.empty()) {
    throw ast_ops::arity_error("at least one argument is required", 1, 0);
  }
  return std::accumulate(vals.begin(), vals.end(), 0.0) / vals.size();
}

}  // namespace builtins