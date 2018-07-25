#include "ast_ops/ast_dump.h"
#include "ast_ops/eval/eval.h"
#include "ast_ops/eval/exceptions.h"
#include "ast_ops/pretty_print.h"
#include "ast_ops/strip_parens.h"
#include "error_handling.h"
#include "mparse/error.h"
#include "mparse/parser.h"
#include "mparse/source_map.h"
#include "scope_helpers.h"
#include <iostream>
#include <utility>

using namespace std::literals;

namespace {

void print_help(std::string_view prog_name) {
  prog_name.remove_prefix(std::min(prog_name.rfind('\\') + 1, prog_name.size()));
  std::cout << "Usage: " << prog_name << " dump|pretty|eval <expr> [var1=val1 var2=val2 ...]\n\n";
  std::cout << "dump - Print a visualization of the AST.\n";
  std::cout << "pretty - Pretty print the expression.\n";
  std::cout << "eval - Evaluate the expression, using the passed variable definitions.\n";
  std::exit(1);
}

auto parse_diag(std::string_view input) {
  try {
    mparse::source_map smap;
    mparse::ast_node_ptr ast = mparse::parse(input, &smap);
    return std::make_pair(std::move(ast), std::move(smap));
  } catch (const mparse::syntax_error& err) {
    handle_syntax_error(err, input);
    std::exit(1);
  }
}

}  // namespace


int main(int argc, const char* const* argv) {
  if (argc < 3) {
    print_help(argv[0]);
  }

  std::string_view cmd = argv[1];
  if (cmd != "dump" && cmd != "eval" && cmd != "pretty" && cmd != "strip") {
    print_help(argv[0]);
  }

  std::string_view input = argv[2];
  auto [ast, smap] = parse_diag(input);

  if (cmd == "dump") {
    ast_ops::dump_ast(ast.get(), &smap);
  } else if (cmd == "pretty") {
    std::cout << ast_ops::pretty_print(ast.get()) << "\n";
  } else if (cmd == "strip") {
    ast_ops::strip_parens(ast);
    std::cout << ast_ops::pretty_print(ast.get()) << "\n";
  } else if (cmd == "eval") {
    ast_ops::var_scope vscope = {
      { "e", 2.718281828459045 },
      { "pi", 3.141592653589793 },
      { "tau", 6.283185307179586 }
    };

    ast_ops::func_scope fscope;
    fscope.set_binding("sin", [] (double x) {
      return std::sin(x);
    });

    parse_vardefs(vscope, argc - 3, argv + 3);

    try {
      std::cout << ast_ops::eval(ast.get(), vscope, fscope) << '\n';
    } catch (const ast_ops::eval_error& err) {
      handle_math_error(err, smap, input);
      return 1;
    }
  }
}