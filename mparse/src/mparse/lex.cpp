#include "lex.h"

using namespace std::literals;

namespace {

// locale-independent, UTF-8
constexpr auto whitespace = " \f\n\r\t\v\u00a0\u1680\u180e\u2000\u2001\u2002\u2003\u2004"
  "\u2005\u2006\u2007\u2008\u2009\u200a\u2028\u2029\u202f\u205f\u3000\ufeff"sv;

constexpr auto delims = "+-*/^()"sv;

bool is_digit(char ch) {
  return ch >= '0' && ch <= '9';
}

}  // namespace

namespace mparse {

token get_token(source_stream& stream) {
  stream.eat_while(whitespace);

  source_stream::pos_type token_start = stream.pos();

  if (stream.eof()) {
    return token{
      token_type::eof,
      token_start,
      stream.token(token_start)
    };
  }

  if (stream.eat_one_of(delims)) {
    return token{
      token_type::delim,
      token_start,
      stream.token(token_start)
    };
  }

  if (stream.eat_while(is_digit)) {
    if (stream.eat('.')) {
      stream.eat_while(is_digit);
    }
    return token{
      token_type::literal,
      token_start,
      stream.token(token_start)
    };
  }

  if (stream.eat('.')) {
    if (stream.eat_while(is_digit)) {
      return token{
        token_type::literal,
        token_start,
        stream.token(token_start)
      };
    }

    return token{
      token_type::unknown,
      token_start,
      stream.token(token_start)
    };
  }

  // give up
  stream.next();
  return token{
    token_type::unknown,
    token_start,
    stream.token(token_start)
  };
}

}  // namespace mparse