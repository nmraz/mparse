#include "ast_ops/ast_dump.h"
#include "ast_ops/eval.h"
#include "loc_printing.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/error.h"
#include "mparse/parser.h"
#include "mparse/source_map.h"
#include "ast_ops/pretty_print.h"
#include <cstdlib>
#include <iostream>
#include <utility>

using namespace std::literals;

namespace {

void print_help(std::string_view prog_name) {
  prog_name = prog_name.substr(prog_name.rfind('\\') + 1);
  std::cout << "Usage: " << prog_name << " dump|eval|pretty <expr>\n";
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
    try {
      std::cout << ast_ops::eval(ast.get()) << '\n';
    } catch (const ast_ops::eval_error& err) {
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

      return 1;
    }
  }
}