#include "helpers.h"

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

} // namespace

void parse_vardefs(ast_ops::var_scope& vscope, int argc,
                   const char* const* argv) {
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