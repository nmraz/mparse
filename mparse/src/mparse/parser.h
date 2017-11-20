#pragma once

#include "mparse/ast/abstract_syntax_tree.h"
#include "mparse/lex.h"
#include "mparse/source_stream.h"

namespace mparse {

class parser {
public:
  explicit parser(source_stream stream);

  void begin_parse() { get_next_token(); }

  ast_node_ptr parse_root();

  ast_node_ptr parse_add();
  ast_node_ptr parse_mult();
  ast_node_ptr parse_unary();
  ast_node_ptr parse_pow();
  
  ast_node_ptr parse_atom();
  ast_node_ptr consume_literal();
  ast_node_ptr consume_paren();

  const source_stream& stream() const { return stream_; }
  token last_token() const { return last_token_; }

private:
  void get_next_token();

  bool has_delim(std::string_view val) const;
  void error() const;

  source_stream stream_;
  token last_token_;

  const char* expected_type_ = "";  // used for informative error messages
};

ast_node_ptr parse(std::string_view source);

}  // namespace mparse