#include "lex.h"

using namespace std::literals;

namespace {

constexpr auto delims = "+-*/^()|,"sv;

// locale-independent, UTF-8
constexpr auto whitespace =
    " \f\n\r\t\v\u00a0\u1680\u180e\u2000\u2001\u2002\u2003\u2004"
    "\u2005\u2006\u2007\u2008\u2009\u200a\u2028\u2029\u202f\u205f\u3000\ufeff"sv;

constexpr bool is_digit(char ch) {
  return ch >= '0' && ch <= '9';
}

constexpr bool is_alpha(char ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

} // namespace

namespace mparse {

token get_token(source_stream& stream) {
  stream.eat_while(whitespace);

  source_stream::pos_type token_start = stream.pos();

  if (stream.eof()) {
    return {.type = token_type::eof,
            .loc = token_start,
            .val = stream.token(token_start)};
  }

  if (stream.eat_one_of(delims)) {
    return {.type = token_type::delim,
            .loc = token_start,
            .val = stream.token(token_start)};
  }

  if (stream.eat_while(is_digit)) {
    if (stream.eat('.')) {
      stream.eat_while(is_digit);
    }
    return {.type = token_type::literal,
            .loc = token_start,
            .val = stream.token(token_start)};
  }

  if (stream.eat('.')) {
    if (stream.eat_while(is_digit)) {
      return {.type = token_type::literal,
              .loc = token_start,
              .val = stream.token(token_start)};
    }

    return {.type = token_type::unknown,
            .loc = token_start,
            .val = stream.token(token_start)};
  }

  if (stream.eat(is_alpha)) {
    stream.eat_while([](char ch) { return is_alpha(ch) || is_digit(ch); });

    return {.type = token_type::ident,
            .loc = token_start,
            .val = stream.token(token_start)};
  }

  // give up
  stream.next();
  return {.type = token_type::unknown,
          .loc = token_start,
          .val = stream.token(token_start)};
}

} // namespace mparse