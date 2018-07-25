#include "scope_helpers.h"

#include "mparse/lex.h"
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <system_error>

using namespace std::literals;

namespace {

std::string accumulate_argv(int argc, const char* const* argv) {
  std::string ret;
  for (int i = 0; i < argc; i++) {
    ret += " "s + argv[i];
  }
  return ret;
}

template<typename F>
constexpr auto wrap_errno(F func) {
  return [func](double x) {
    errno = 0;
    double ret = func(x);
    if (errno) {
      throw std::system_error(errno, std::generic_category());
    }
    return ret;
  };
}

}  // namespace




ast_ops::var_scope default_var_scope() {
  return {
    { "e", 2.718281828459045 },
    { "pi", 3.141592653589793 },
    { "tau", 6.283185307179586 }
  };
}

ast_ops::func_scope default_func_scope() {
  ast_ops::func_scope scope;

  scope.set_binding("sin", wrap_errno([] (double x) { return std::sin(x); }));
  scope.set_binding("cos", wrap_errno([] (double x) { return std::cos(x); }));
  scope.set_binding("tan", wrap_errno([] (double x) { return std::tan(x); }));
  
  constexpr auto arcsin = wrap_errno([] (double x) { return std::asin(x); });
  constexpr auto arccos = wrap_errno([] (double x) { return std::acos(x); });
  constexpr auto arctan = wrap_errno([] (double x) { return std::atan(x); });

  scope.set_binding("arcsin", arcsin);
  scope.set_binding("asin", arcsin);
  scope.set_binding("arccos", arccos);
  scope.set_binding("acos", arccos);
  scope.set_binding("arctan", arctan);
  scope.set_binding("atan", arctan);


  scope.set_binding("sinh", wrap_errno([] (double x) { return std::sinh(x); }));
  scope.set_binding("cosh", wrap_errno([] (double x) { return std::cosh(x); }));
  scope.set_binding("tanh", wrap_errno([] (double x) { return std::tanh(x); }));

  constexpr auto arcsinh = wrap_errno([] (double x) { return std::asinh(x); });
  constexpr auto arccosh = wrap_errno([] (double x) { return std::acosh(x); });
  constexpr auto arctanh = wrap_errno([] (double x) { return std::atanh(x); });

  scope.set_binding("arcsinh", arcsinh);
  scope.set_binding("asinh", arcsinh);
  scope.set_binding("arccosh", arccosh);
  scope.set_binding("acosh", arccosh);
  scope.set_binding("arctanh", arctanh);
  scope.set_binding("atanh", arctanh);


  scope.set_binding("exp", wrap_errno([] (double x) { return std::exp(x); }));
  scope.set_binding("ln", wrap_errno([] (double x) { return std::log(x); }));
  scope.set_binding("log", [] (double base, double val) {
    if (val <= 0) {
      throw std::domain_error("argument out of domain");
    }
    if (base <= 0 || base == 1) {
      throw std::domain_error("base out of domain");
    }
    return std::log(val) / std::log(base);
  });

  scope.set_binding("sqrt", wrap_errno([] (double x) { return std::sqrt(x); }));
  scope.set_binding("cbrt", wrap_errno([] (double x) { return std::cbrt(x); }));
  scope.set_binding("nroot", [] (double n, double val) {
    if (val < 0 && std::fmod(n, 2) != 1) {
      throw std::domain_error("taking non-odd nth-root of negative number");
    }
    if (n == 0) {
      throw std::domain_error("taking zero-th root of number");
    }
    if (val == 0 && n < 0) {
      throw std::domain_error("taking negative root of zero");
    }

    if (val < 0) {
      return -std::pow(-val, 1 / n);
    }
    return std::pow(val, 1 / n);
  });

  return scope;
}


void parse_vardefs(ast_ops::var_scope& vscope, int argc, const char* const* argv) {
  std::string input = accumulate_argv(argc, argv);
  mparse::source_stream stream(input);

  mparse::token last_tok;

  do {
    std::string name;
    double val;

    last_tok = get_token(stream);
    if (last_tok.type != mparse::token_type::ident) {
      continue;
    }
    name = last_tok.val;

    last_tok = get_token(stream);
    if (last_tok.val != "=") {
      continue;
    }

    last_tok = get_token(stream);
    if (last_tok.type != mparse::token_type::literal) {
      continue;
    }
    val = std::strtod(last_tok.val.data(), nullptr);

    vscope.set_binding(std::move(name), val);
  } while (last_tok.type != mparse::token_type::eof);
}