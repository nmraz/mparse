#include "scope_helpers.h"

#include "builtins.h"
#include "mparse/lex.h"
#include <cstdlib>
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
      throw ast_ops::func_arg_error(std::error_code(errno, std::generic_category()).message(), { 0 });
    }
    return ret;
  };
}

}  // namespace


ast_ops::var_scope default_var_scope() {
  return {
    { "e", builtins::e },
    { "pi", builtins::pi },
    { "tau", builtins::tau }
  };
}

ast_ops::func_scope default_func_scope() {
  ast_ops::func_scope scope;

  scope.set_binding("mod", builtins::mod);

  scope.set_binding("sin", wrap_errno(builtins::sin));
  scope.set_binding("cos", wrap_errno(builtins::cos));
  scope.set_binding("tan", wrap_errno(builtins::tan));
  
  auto arcsin = wrap_errno(builtins::asin);
  auto arccos = wrap_errno(builtins::acos);
  auto arctan = wrap_errno(builtins::atan);

  scope.set_binding("arcsin", arcsin);
  scope.set_binding("asin", arcsin);
  scope.set_binding("arccos", arccos);
  scope.set_binding("acos", arccos);
  scope.set_binding("arctan", arctan);
  scope.set_binding("atan", arctan);


  scope.set_binding("sinh", wrap_errno(builtins::sinh));
  scope.set_binding("cosh", wrap_errno(builtins::cosh));
  scope.set_binding("tanh", wrap_errno(builtins::tanh));

  auto arcsinh = wrap_errno(builtins::asinh);
  auto arccosh = wrap_errno(builtins::acosh);
  auto arctanh = wrap_errno(builtins::atanh);

  scope.set_binding("arcsinh", arcsinh);
  scope.set_binding("asinh", arcsinh);
  scope.set_binding("arccosh", arccosh);
  scope.set_binding("acosh", arccosh);
  scope.set_binding("arctanh", arctanh);
  scope.set_binding("atanh", arctanh);


  scope.set_binding("exp", wrap_errno(builtins::exp));
  scope.set_binding("ln", wrap_errno(builtins::ln));
  scope.set_binding("log", builtins::log);

  scope.set_binding("sqrt", wrap_errno(builtins::sqrt));
  scope.set_binding("cbrt", wrap_errno(builtins::cbrt));
  scope.set_binding("nroot", builtins::nroot);

  
  scope.set_binding("min", builtins::min);
  scope.set_binding("max", builtins::max);
  scope.set_binding("avg", builtins::avg);

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