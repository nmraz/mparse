#pragma once

#include <string_view>

namespace mparse::lex_util {

using namespace std::string_view_literals;


// locale-independent, UTF-8
constexpr auto whitespace = " \f\n\r\t\v\u00a0\u1680\u180e\u2000\u2001\u2002\u2003\u2004"
  "\u2005\u2006\u2007\u2008\u2009\u200a\u2028\u2029\u202f\u205f\u3000\ufeff"sv;

constexpr bool is_digit(char ch) {
  return ch >= '0' && ch <= '9';
}

constexpr bool is_alpha(char ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

}  // namespace mparse::lex_util