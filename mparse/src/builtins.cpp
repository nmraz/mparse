#include "builtins.h"

#include "ast_ops/eval/eval_error.h"
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <string>
#include <system_error>

using namespace std::literals;

namespace builtins {
namespace {

template<typename F>
ast_ops::number check_errno(F func) {
  errno = 0;
  ast_ops::number ret = func();
  if (errno) {
    std::string msg = std::error_code(errno, std::generic_category()).message();
    if (errno == EDOM) {
      throw ast_ops::func_arg_error(msg, { 0 });
    }
    throw std::runtime_error(msg);
  }
  return ret;
}

}  // namespace


double mod(double a, double b) {
  if (b == 0) {
    throw ast_ops::func_arg_error("mod by zero", { 1 });
  }
  return std::fmod(a, b);
}


ast_ops::number sin(ast_ops::number x) {
  return check_errno([&] {
    return std::sin(x);
  });
}

ast_ops::number cos(ast_ops::number x) {
  return check_errno([&] {
    return std::cos(x);
  });
}

ast_ops::number tan(ast_ops::number x) {
  return check_errno([&] {
    return std::tan(x);
  });
}


ast_ops::number asin(ast_ops::number x) {
  return check_errno([&] {
    return std::asin(x);
  });
}

ast_ops::number acos(ast_ops::number x) {
  return check_errno([&] {
    return std::acos(x);
  });
}

ast_ops::number atan(ast_ops::number x) {
  return check_errno([&] {
    return std::atan(x);
  });
}


ast_ops::number sinh(ast_ops::number x) {
  return check_errno([&] {
    return std::sinh(x);
  });
}

ast_ops::number cosh(ast_ops::number x) {
  return check_errno([&] {
    return std::cosh(x);
  });
}

ast_ops::number tanh(ast_ops::number x) {
  return check_errno([&] {
    return std::tanh(x);
  });
}


ast_ops::number asinh(ast_ops::number x) {
  return check_errno([&] {
    return std::asinh(x);
  });
}

ast_ops::number acosh(ast_ops::number x) {
  return check_errno([&] {
    return std::acosh(x);
  });
}

ast_ops::number atanh(ast_ops::number x) {
  return check_errno([&] {
    return std::atanh(x);
  });
}


ast_ops::number exp(ast_ops::number x) {
  return check_errno([&] {
    return std::exp(x);
  });
}

ast_ops::number ln(ast_ops::number x) {
  return check_errno([&] {
    return std::log(x);
  });
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


ast_ops::number sqrt(ast_ops::number x) {
  return check_errno([&] {
    return std::sqrt(x);
  });
}

ast_ops::number cbrt(ast_ops::number x) {
  return check_errno([&] {
    return std::pow(x, 1 / 3);
  });
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

ast_ops::number avg(std::vector<ast_ops::number> vals) {
  if (vals.empty()) {
    throw ast_ops::arity_error("at least one argument is required", 1, 0);
  }
  return std::accumulate(vals.begin(), vals.end(), 0i) / static_cast<double>(vals.size());
}

}  // namespace builtins