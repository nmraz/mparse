#include "source_stream.h"

#include <cassert>

namespace mparse {

source_stream::source_type source_stream::token(pos_type token_start) const {
  assert(pos_ >= token_start && "Token should begin before it ends");
  return source_.substr(token_start, pos_ - token_start);
}


source_stream::int_type source_stream::next() {
  if (eof()) {
    return traits_type::eof();
  }
  return source_[pos_++];
}

source_stream::int_type source_stream::peek(int lookahead) const {
  pos_type peek_pos = pos_ + lookahead;
  if (peek_pos >= source_.size() || peek_pos < 0) {
    return traits_type::eof();
  }
  return source_[peek_pos];
}

bool source_stream::eat(char_type ch) {
  return eat([=](char peeked) { return peeked == ch; });
}

bool source_stream::eat_one_of(source_type chars) {
  return eat(
      [&](char peeked) { return chars.find(peeked) != source_type::npos; });
}

source_stream::pos_type source_stream::eat_while(source_type chars) {
  return eat_while(
      [&](char peeked) { return chars.find(peeked) != source_type::npos; });
}

} // namespace mparse