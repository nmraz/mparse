#include "scope_helpers.h"

#include "builtins.h"
#include "mparse/lex.h"
#include <string>

using namespace std::literals;

namespace {

std::string accumulate_argv(int argc, const char* const* argv) {
  std::string ret;
  for (int i = 0; i < argc; i++) {
    ret += " "s + argv[i];
  }
  return ret;
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

  scope.set_binding("sin", builtins::sin);
  scope.set_binding("cos", builtins::cos);
  scope.set_binding("tan", builtins::tan);
  
  scope.set_binding("arcsin", builtins::asin);
  scope.set_binding("asin", builtins::asin);
  scope.set_binding("arccos", builtins::acos);
  scope.set_binding("acos", builtins::acos);
  scope.set_binding("arctan", builtins::atan);
  scope.set_binding("atan", builtins::atan);


  scope.set_binding("sinh", builtins::sinh);
  scope.set_binding("cosh", builtins::cosh);
  scope.set_binding("tanh", builtins::tanh);

  scope.set_binding("arcsinh", builtins::asinh);
  scope.set_binding("asinh", builtins::asinh);
  scope.set_binding("arccosh", builtins::acosh);
  scope.set_binding("acosh", builtins::acosh);
  scope.set_binding("arctanh", builtins::atanh);
  scope.set_binding("atanh", builtins::atanh);


  scope.set_binding("exp", builtins::exp);
  scope.set_binding("ln", builtins::ln);
  scope.set_binding("log", builtins::log);

  scope.set_binding("sqrt", builtins::sqrt);
  scope.set_binding("cbrt", builtins::cbrt);
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