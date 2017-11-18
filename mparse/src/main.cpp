#include "ast_dump.h"
#include "eval.h"
#include "loc_printing.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/error.h"
#include "mparse/parser.h"
#include "pretty_print.h"
#include <cstdlib>
#include <iostream>

using namespace std::literals;

namespace {

void print_help(std::string_view prog_name) {
  std::cout << "Usage: " << prog_name << " dump|eval|pretty <expr>";
  std::exit(1);
}

mparse::abstract_syntax_tree parse_diag(std::string_view input) {
  try {
    return mparse::parse(input);
  } catch (const mparse::syntax_error& err) {
    std::cout << "Syntax error: " << err.what() << "\n\n";
    print_loc(err.where(), input);
    std::exit(1);
  }
}

}

int main(int argc, const char* const* argv) {
  if (argc < 3) {
    print_help(argv[0]);
  }

  std::string_view cmd = argv[1];
  if (cmd != "dump" && cmd != "eval" && cmd != "pretty") {
    print_help(argv[0]);
  }

  std::string_view input = argv[2];
  mparse::abstract_syntax_tree ast = parse_diag(input);

  if (cmd == "dump") {
    dump_ast(ast.root());
  } else if (cmd == "pretty") {
    std::cout << pretty_print(ast.root()) << "\n";
  } else if (cmd == "eval") {
    try {
      std::cout << eval(ast.root()) << '\n';
    } catch (const eval_error& err) {
      std::cout << "Math error: " << err.what() << "\n\n";

      switch (err.code()) {
      case eval_errc::div_by_zero:
        print_loc(static_cast<const mparse::binary_op_node*>(err.node())->rhs()->source_loc(), input);
        break;
      case eval_errc::bad_pow:
      {
        const mparse::binary_op_node* node = static_cast<const mparse::binary_op_node*>(err.node());

        print_locs({ node->lhs()->source_loc(), node->rhs()->source_loc() }, input);
      }
      default:
        break;
      }

      return 1;
    }
  }
}