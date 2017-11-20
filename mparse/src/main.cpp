#include "ast_dump.h"
#include "eval.h"
#include "loc_printing.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/error.h"
#include "mparse/parser.h"
#include "mparse/source_map.h"
#include "pretty_print.h"
#include <cstdlib>
#include <iostream>
#include <utility>

using namespace std::literals;

namespace {

void print_help(std::string_view prog_name) {
  std::cout << "Usage: " << prog_name << " dump|eval|pretty <expr>";
  std::exit(1);
}

auto parse_diag(std::string_view input) {
  try {
    mparse::source_map smap;
    mparse::ast_node_ptr ast = mparse::parse(input, &smap);
    return std::make_pair(std::move(ast), std::move(smap));
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
  auto [ast, smap] = parse_diag(input);

  if (cmd == "dump") {
    dump_ast(ast.get(), smap);
  } else if (cmd == "pretty") {
    std::cout << pretty_print(ast.get()) << "\n";
  } else if (cmd == "eval") {
    try {
      std::cout << eval(ast.get()) << '\n';
    } catch (const eval_error& err) {
      std::cout << "Math error: " << err.what() << "\n\n";

      const mparse::binary_op_node* node = static_cast<const mparse::binary_op_node*>(err.node());

      switch (err.code()) {
      case eval_errc::div_by_zero:
        print_loc(smap.find_primary_loc(node->rhs()), input);
        break;
      case eval_errc::bad_pow:
        print_locs({ smap.find_primary_loc(node->lhs()), smap.find_primary_loc(node->rhs()) }, input);
      default:
        break;
      }

      return 1;
    }
  }
}