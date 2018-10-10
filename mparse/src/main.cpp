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
#include "util/span.h"
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <utility>

using namespace std::literals;

namespace {

using subcommand_func =
    std::function<void(mparse::ast_node_ptr, mparse::source_map,
                       std::string_view, util::span<const char* const>)>;

struct subcommand {
  std::string_view desc;
  subcommand_func func;
};

using command_map = std::map<std::string, subcommand, std::less<>>;

void print_help(std::string_view prog_name, const command_map& commands) {
  prog_name.remove_prefix(
      std::min(prog_name.find_last_of("\\/") + 1, prog_name.size()));

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
    return std::pair{std::move(ast), std::move(smap)};
  } catch (const mparse::syntax_error& err) {
    handle_syntax_error(err, input);
    std::exit(1);
  }
}


void cmd_dump(mparse::ast_node_ptr ast, mparse::source_map smap,
              std::string_view, util::span<const char* const>) {
  ast_ops::dump_ast(ast.get(), &smap);
}

void cmd_pretty(mparse::ast_node_ptr ast, mparse::source_map, std::string_view,
                util::span<const char* const>) {
  std::cout << ast_ops::pretty_print(ast.get()) << "\n";
}

void cmd_strip(mparse::ast_node_ptr ast, mparse::source_map, std::string_view,
               util::span<const char* const>) {
  ast_ops::strip_parens(ast);
  std::cout << ast_ops::pretty_print(ast.get()) << "\n";
}

void cmd_paren(mparse::ast_node_ptr ast, mparse::source_map, std::string_view,
               util::span<const char* const>) {
  ast_ops::strip_parens(ast);
  ast_ops::insert_parens(ast);
  std::cout << ast_ops::pretty_print(ast.get()) << "\n";
}

void cmd_eval(mparse::ast_node_ptr ast, mparse::source_map smap,
              std::string_view input, util::span<const char* const> argv) {
  auto vscope = ast_ops::builtin_var_scope();
  parse_vardefs(vscope, argv);

  try {
    auto result =
        ast_ops::eval(ast.get(), vscope, ast_ops::builtin_func_scope());
    print_number(std::cout, result) << '\n';
  } catch (const ast_ops::eval_error& err) {
    handle_math_error(err, smap, input);
    std::exit(1);
  }
}

} // namespace


int main(int argc, const char* const* argv) {
  std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1);

  command_map commands = {
      {"dump", {"Print a visualization of the AST.", cmd_dump}},
      {"pretty", {"Pretty print the expression.", cmd_pretty}},
      {"strip",
       {"Pretty print the expression, with superfluous parentheses removed.",
        cmd_strip}},
      {"paren",
       {"Pretty print the expression, showing all implicit parentheses.",
        cmd_paren}},
      {"eval",
       {"Evaluate the expression, using passed variable definitions of the "
        "form 'var1=val1 var2=val2'.",
        cmd_eval}},
  };

  if (argc < 3) {
    print_help(argv[0], commands);
  }

  std::string_view input = argv[2];
  auto [ast, smap] = parse_diag(input);

  if (auto it = commands.find(argv[1]); it != commands.end()) {
    it->second.func(std::move(ast), std::move(smap), input,
                    util::span{argv + 3, argc - 3});
  } else {
    print_help(argv[0], commands);
  }
}