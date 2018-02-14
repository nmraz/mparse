#include "ast_ops/ast_dump.h"
#include "ast_ops/eval/eval.h"
#include "ast_ops/pretty_print.h"
#include "loc_printing.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/error.h"
#include "mparse/parser.h"
#include "mparse/source_map.h"
#include "parse_vardefs.h"
#include <cstdlib>
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
    std::cout << "Syntax error: " << err.what() << "\n\n";
    print_loc(input, err.where());
    std::exit(1);
  }
}

void print_math_error(const ast_ops::eval_error& err, const mparse::source_map& smap, std::string_view input) {
  std::cout << "Math error: " << err.what() << "\n\n";

  const mparse::binary_op_node* node = static_cast<const mparse::binary_op_node*>(err.node());

  switch (err.code()) {
  case ast_ops::eval_errc::div_by_zero:
    print_loc(input, smap.find_primary_loc(node->rhs()));
    break;
  case ast_ops::eval_errc::bad_pow:
    print_locs(input, { smap.find_primary_loc(node->lhs()), smap.find_primary_loc(node->rhs()) });
    break;
  case ast_ops::eval_errc::unbound_var:
    print_loc(input, smap.find_primary_loc(err.node()));
  default:
    break;
  }
}

}  // namespace


int main(int argc, const char* const* argv) {
  if (argc < 3) {
    print_help(argv[0]);
  }

  std::string_view cmd = argv[1];
  if (cmd != "dump" && cmd != "eval" && cmd != "pretty") {
    print_help(argv[0]);
  }

  std::string_view input = argv[2];
  auto [ast, smap] = parse_diag(input);

  if (cmd == "dump") {
    ast_ops::dump_ast(ast.get(), &smap);
  } else if (cmd == "pretty") {
    std::cout << ast_ops::pretty_print(ast.get()) << "\n";
  } else if (cmd == "eval") {
    ast_ops::eval_scope scope = {
      { "e", 2.71828183 },
      { "pi", 3.14159265 },
      { "tau", 6.28318531 }
    };

    parse_vardefs(scope, argc - 3, argv + 3);

    try {
      std::cout << ast_ops::eval(ast.get(), scope) << '\n';
    } catch (const ast_ops::eval_error& err) {
      print_math_error(err, smap, input);
      return 1;
    }
  }
}