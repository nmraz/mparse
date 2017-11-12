#pragma once

#include "mparse/source_stream.h"

namespace mparse {

enum class token_type {
  literal,  // number literal
  delim,  // delimiter (operator)
  eof,  // end of input
  unknown  // error
};

struct token {
  token_type type;
  std::size_t loc;
  std::string_view val;
};

token get_token(source_stream& stream);

}  // namespace mparse