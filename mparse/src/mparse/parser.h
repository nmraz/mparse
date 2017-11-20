#pragma once

#include "mparse/ast/ast_node.h"
#include "mparse/lex.h"
#include "mparse/source_stream.h"

namespace mparse {

class parser {
public:
  explicit parser(source_stream& stream);

  void begin_parse();
  void end_parse();

  ast_node_ptr parse_root();

  ast_node_ptr parse_add();
  ast_node_ptr parse_mult();
  ast_node_ptr parse_unary();
  ast_node_ptr parse_pow();
  
  ast_node_ptr parse_atom();
  ast_node_ptr consume_literal();
  ast_node_ptr consume_paren();

  void get_next_token();
  token cur_token() const { return cur_token_; }

private:
  bool has_delim(std::string_view val) const;
  void error() const;

  source_stream& stream_;
  token cur_token_;

  const char* expected_type_ = "";  // used for informative error messages
};

ast_node_ptr parse(std::string_view source);

}  // namespace mparse