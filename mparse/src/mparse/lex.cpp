#include "lex.h"

#include "mparse/lex_util.h"

using namespace std::literals;

namespace {

constexpr auto delims = "+-*/^()|,"sv;

} // namespace

namespace mparse {

token get_token(source_stream& stream) {
  stream.eat_while(lex_util::whitespace);

  source_stream::pos_type token_start = stream.pos();

  if (stream.eof()) {
    return token{token_type::eof, token_start, stream.token(token_start)};
  }

  if (stream.eat_one_of(delims)) {
    return token{token_type::delim, token_start, stream.token(token_start)};
  }

  if (stream.eat_while(lex_util::is_digit)) {
    if (stream.eat('.')) {
      stream.eat_while(lex_util::is_digit);
    }
    return token{token_type::literal, token_start, stream.token(token_start)};
  }

  if (stream.eat('.')) {
    if (stream.eat_while(lex_util::is_digit)) {
      return token{token_type::literal, token_start, stream.token(token_start)};
    }

    return token{token_type::unknown, token_start, stream.token(token_start)};
  }

  if (stream.eat(lex_util::is_alpha)) {
    stream.eat_while([](char ch) {
      return lex_util::is_alpha(ch) || lex_util::is_digit(ch);
    });

    return token{token_type::ident, token_start, stream.token(token_start)};
  }

  // give up
  stream.next();
  return token{token_type::unknown, token_start, stream.token(token_start)};
}

} // namespace mparse