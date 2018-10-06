#include "helpers.h"

#include "mparse/lex.h"
#include <charconv>
#include <cmath>
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


double to_precision(double num, double round_prec) {
  if (std::abs(num - std::nextafter(num, 0)) > 1 / round_prec) {
    return num;
  }
  return std::round(num * round_prec) / round_prec;
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

    auto status = std::from_chars(last_tok.val.data(),
                                  last_tok.val.data() + last_tok.val.size(),
                                  val, std::chars_format::fixed);

    if (status.ec != std::errc{}) {
      continue;
    }

    vscope.set_binding(std::move(name), val);
  } while (last_tok.type != mparse::token_type::eof);
}

std::ostream& print_number(std::ostream& stream, ast_ops::number num,
                           double round_prec) {
  num.real(to_precision(num.real(), round_prec));
  num.imag(to_precision(num.imag(), round_prec));

  if (num == 0.0) {
    return stream << 0.0;
  }
  if (num == -1i) {
    return stream << "-i";
  }

  if (num.imag() == 0.0) {
    return stream << num.real();
  }

  if (num.real() != 0.0) {
    stream << num.real() << " ";
    if (num.imag() < 0.0) {
      stream << "-";
      num.imag(-num.imag());
    } else {
      stream << "+";
    }
    stream << " ";
  }
  if (num.imag() != 1.0) {
    stream << num.imag() << "*";
  }
  return stream << "i";
}