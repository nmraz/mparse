#include "builtins.h"

#include "ast_ops/eval/eval_error.h"
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <string>

using namespace std::literals;

namespace builtins {
namespace {

constexpr auto domain_err_msg = "argument out of domain"sv;

template<typename F>
auto check_domain(F func) {
  errno = 0;
  auto ret = func();
  if (errno) {
    if (errno == EDOM) {
      throw ast_ops::func_arg_error(domain_err_msg, { 0 });
    }
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
  return check_domain([&] {
    return std::sin(x);
  });
}

ast_ops::number cos(ast_ops::number x) {
  return check_domain([&] {
    return std::cos(x);
  });
}

ast_ops::number tan(ast_ops::number x) {
  return check_domain([&] {
    return std::tan(x);
  });
}


ast_ops::number asin(ast_ops::number x) {
  return check_domain([&] {
    return std::asin(x);
  });
}

ast_ops::number acos(ast_ops::number x) {
  return check_domain([&] {
    return std::acos(x);
  });
}

ast_ops::number atan(ast_ops::number x) {
  return check_domain([&] {
    return std::atan(x);
  });
}


ast_ops::number sinh(ast_ops::number x) {
  return check_domain([&] {
    return std::sinh(x);
  });
}

ast_ops::number cosh(ast_ops::number x) {
  return check_domain([&] {
    return std::cosh(x);
  });
}

ast_ops::number tanh(ast_ops::number x) {
  return check_domain([&] {
    return std::tanh(x);
  });
}


ast_ops::number asinh(ast_ops::number x) {
  return check_domain([&] {
    return std::asinh(x);
  });
}

ast_ops::number acosh(ast_ops::number x) {
  return check_domain([&] {
    return std::acosh(x);
  });
}

ast_ops::number atanh(ast_ops::number x) {
  return check_domain([&] {
    return std::atanh(x);
  });
}


ast_ops::number exp(ast_ops::number x) {
  return check_domain([&] {
    return std::exp(x);
  });
}

ast_ops::number ln(ast_ops::number x) {
  if (x == 0.0) {
    throw ast_ops::func_arg_error(domain_err_msg, { 0 });
  }
  return check_domain([&] {
    return std::log(x);
  });
}

ast_ops::number log(ast_ops::number base, ast_ops::number val) {
  if (val == 0.0) {
    throw ast_ops::func_arg_error(domain_err_msg, { 1 });
  }
  if (base == 0.0 || base == 1.0) {
    throw ast_ops::func_arg_error("base out of domain", { 0 });
  }
  return std::log(val) / std::log(base);
}


ast_ops::number sqrt(ast_ops::number x) {
  return check_domain([&] {
    return std::sqrt(x);
  });
}

double cbrt(double x) {
  return check_domain([&] {
    return std::cbrt(x);
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


double re(ast_ops::number x) {
  return x.real();
}

double im(ast_ops::number x) {
  return x.imag();
}

double arg(ast_ops::number x) {
  return check_domain([&] {
    return std::arg(x);
  });
}

ast_ops::number conj(ast_ops::number x) {
  return std::conj(x);
}


double floor(double x) {
  return check_domain([&] {
    return std::floor(x);
  });
}

double ceil(double x) {
  return check_domain([&] {
    return std::ceil(x);
  });
}

double round(double x) {
  return check_domain([&] {
    return std::round(x);
  });
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