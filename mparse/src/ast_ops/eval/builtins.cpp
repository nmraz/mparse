#include "builtins.h"

#include "ast_ops/eval/eval_error.h"
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <string>

using namespace std::literals;

namespace ast_ops {
namespace builtins {
namespace {

constexpr auto domain_err_msg = "argument out of domain"sv;

template <typename F>
auto check_domain(F func) {
  errno = 0;
  auto ret = func();
  if (errno) {
    if (errno == EDOM) {
      throw func_arg_error(domain_err_msg, {0});
    }
  }
  return ret;
}

} // namespace


number sin(number x) {
  return check_domain([&] { return std::sin(x); });
}

number cos(number x) {
  return check_domain([&] { return std::cos(x); });
}

number tan(number x) {
  return check_domain([&] { return std::tan(x); });
}


number asin(number x) {
  return check_domain([&] { return std::asin(x); });
}

number acos(number x) {
  return check_domain([&] { return std::acos(x); });
}

number atan(number x) {
  return check_domain([&] { return std::atan(x); });
}


number sinh(number x) {
  return check_domain([&] { return std::sinh(x); });
}

number cosh(number x) {
  return check_domain([&] { return std::cosh(x); });
}

number tanh(number x) {
  return check_domain([&] { return std::tanh(x); });
}


number asinh(number x) {
  return check_domain([&] { return std::asinh(x); });
}

number acosh(number x) {
  return check_domain([&] { return std::acosh(x); });
}

number atanh(number x) {
  return check_domain([&] { return std::atanh(x); });
}


number exp(number x) {
  return check_domain([&] { return std::exp(x); });
}

number ln(number x) {
  if (x == 0.0) {
    throw func_arg_error(domain_err_msg, {0});
  }
  return check_domain([&] { return std::log(x); });
}

number log(number base, number val) {
  if (val == 0.0) {
    throw func_arg_error(domain_err_msg, {1});
  }
  if (base == 0.0 || base == 1.0) {
    throw func_arg_error("base out of domain", {0});
  }
  return std::log(val) / std::log(base);
}


number sqrt(number x) {
  return check_domain([&] { return std::sqrt(x); });
}

double cbrt(double x) {
  return check_domain([&] { return std::cbrt(x); });
}

double nroot(double n, double val) {
  if (val < 0 && std::fmod(n, 2) != 1) {
    throw func_arg_error("taking non-odd nth-root of negative number", {0, 1});
  }
  if (n == 0) {
    throw func_arg_error("taking zero-th root of number", {0});
  }
  if (val == 0 && n < 0) {
    throw func_arg_error("taking negative root of zero", {0, 1});
  }

  if (val < 0) {
    return -std::pow(-val, 1 / n);
  }
  return std::pow(val, 1 / n);
}


double re(number x) {
  return x.real();
}

double im(number x) {
  return x.imag();
}

double arg(number x) {
  return check_domain([&] { return std::arg(x); });
}

number conj(number x) {
  return std::conj(x);
}


double floor(double x) {
  return check_domain([&] { return std::floor(x); });
}

double ceil(double x) {
  return check_domain([&] { return std::ceil(x); });
}

double round(double x) {
  return check_domain([&] { return std::round(x); });
}


double mod(double a, double b) {
  if (b == 0) {
    throw func_arg_error("mod by zero", {1});
  }
  return std::fmod(a, b);
}


double min(real_func_args vals) {
  if (vals.empty()) {
    throw arity_error("at least one argument is required", 1, 0);
  }
  return *std::min_element(vals.begin(), vals.end());
}

double max(real_func_args vals) {
  if (vals.empty()) {
    throw arity_error("at least one argument is required", 1, 0);
  }
  return *std::max_element(vals.begin(), vals.end());
}

number avg(func_args vals) {
  if (vals.empty()) {
    throw arity_error("at least one argument is required", 1, 0);
  }
  return std::accumulate(vals.begin(), vals.end(), 0i) /
         static_cast<double>(vals.size());
}

} // namespace builtins


var_scope builtin_var_scope() {
  return {
      {"e", builtins::e},
      {"pi", builtins::pi},
      {"tau", builtins::tau},
      {"i", builtins::i},
  };
}

func_scope builtin_func_scope() {
  // clang-format off

  return {
      {"sin", builtins::sin},
      {"cos", builtins::cos},
      {"tan", builtins::tan},

      {"arcsin", builtins::asin},
      {"asin", builtins::asin},
      {"arccos", builtins::acos},
      {"acos", builtins::acos},
      {"arctan", builtins::atan},
      {"atan", builtins::atan},

      {"sinh", builtins::sinh},
      {"cosh", builtins::cosh},
      {"tanh", builtins::tanh},

      {"arcsinh", builtins::asinh},
      {"asinh", builtins::asinh},
      {"arccosh", builtins::acosh},
      {"acosh", builtins::acosh},
      {"arctanh", builtins::atanh},
      {"atanh", builtins::atanh},

      {"exp", builtins::exp},
      {"ln", builtins::ln},
      {"log", builtins::log},

      {"sqrt", builtins::sqrt},
      {"cbrt", builtins::cbrt},
      {"nroot", builtins::nroot},

      {"re", builtins::re},
      {"real", builtins::re},
      {"im", builtins::im},
      {"imag", builtins::im},
      {"arg", builtins::arg},
      {"conj", builtins::conj},

      {"floor", builtins::floor},
      {"ceil", builtins::ceil},
      {"round", builtins::round},

      {"mod", builtins::mod},

      {"min", builtins::min},
      {"max", builtins::max},
      {"avg", builtins::avg},
  };

  // clang-format on
}

} // namespace ast_ops