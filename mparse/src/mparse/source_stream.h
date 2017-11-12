#pragma once

#include <string_view>
#include <utility>

namespace mparse {

class source_stream {
public:
  using char_type = char;
  using traits_type = std::char_traits<char_type>;
  using source_type = std::basic_string_view<char_type>;
  
  using int_type = traits_type::int_type;
  using pos_type = std::size_t;


  explicit constexpr source_stream(source_type source) : source_(source) {}

  source_type source() const { return source_; }
  source_type token(pos_type token_start) const;
  pos_type pos() const { return pos_; } 

  int_type next();
  int_type peek(int lookahead = 0) const;
  int_type cur_char() const { return peek(-1); }
  bool eof() const { return pos_ >= source_.size(); }

  template<typename Pred>
  bool eat(Pred&& pred);
  bool eat(char_type ch);
  bool eat_one_of(source_type chars);

  template<typename Pred>
  pos_type eat_while(Pred&& pred);
  pos_type eat_while(source_type chars);

private:
  source_type source_;
  pos_type pos_ = 0;
};


template<typename Pred>
bool source_stream::eat(Pred&& pred) {
  int_type next_ch = peek();
  if (
    next_ch != traits_type::eof()
    && std::forward<Pred>(pred)(traits_type::to_char_type(next_ch))
  ) {
    pos_++;
    return true;
  }
  return false;
}

template<typename Pred>
source_stream::pos_type source_stream::eat_while(Pred&& pred) {
  pos_type old_pos = pos_;
  while (eat(std::forward<Pred>(pred))) {}
  return pos_ - old_pos;
}

}  // namespace mparse