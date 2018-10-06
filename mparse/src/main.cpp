#include "ast_ops/ast_dump.h"
#include "ast_ops/eval/builtins.h"
#include "ast_ops/eval/eval.h"
#include "ast_ops/eval/eval_error.h"
#include "ast_ops/pretty_print.h"
#include "ast_ops/simplify.h"
#include "error_handling.h"
#include "helpers.h"
#include "mparse/parse_error.h"
#include "mparse/parser.h"
#include "mparse/source_map.h"
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <utility>

using namespace std::literals;

namespace {

using subcommand_func = std::function<int(
    mparse::ast_node_ptr, mparse::source_map, int, const char* const*)>;

struct subcommand {
  std::string_view desc;
  subcommand_func func;
};

using command_map = std::map<std::string, subcommand, std::less<>>;

void print_help(std::string_view prog_name, const command_map& commands) {
  std::cout << "Usage: " << prog_name << " ";

  bool first_iter = true;
  for (const auto& [name, cmd] : commands) {
    if (!first_iter) {
      std::cout << "|";
    }
    first_iter = false;
    std::cout << name;
  }

  std::cout << " <expr> [options]\n\n";

  for (const auto& [name, cmd] : commands) {
    std::cout << name << " - " << cmd.desc << "\n";
  }

  std::exit(2);
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

} // namespace


int main(int argc, const char* const* argv) {
  std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1);
}