#pragma once

#include "mparse/ast/ast_node.h"
#include "mparse/lex.h"
#include "mparse/source_map.h"
#include "mparse/source_stream.h"
#include <vector>

namespace mparse {

class parser {
public:
  parser(source_stream& stream, source_map* smap = nullptr);

  void begin_parse();
  void end_parse();

  ast_node_ptr parse_root();

  ast_node_ptr parse_add();
  ast_node_ptr parse_mult();
  ast_node_ptr parse_unary();
  ast_node_ptr parse_pow();
  
  ast_node_ptr parse_atom();
  ast_node_ptr consume_literal();
  ast_node_ptr consume_ident();
  
  template<typename T>
  ast_node_ptr consume_paren_like(std::string_view term_tok, const char* friendly_name);

  void get_next_token();
  token cur_token() const { return cur_token_; }

private:
  void push_term_tok(std::string_view term_tok);
  void pop_term_tok();

  bool has_term_tok() const;
  bool has_delim(std::string_view val) const;
  void error() const;

  void save_bin_locs(const binary_op_node* node, source_range op_loc);

  source_stream& stream_;
  source_map* smap_;

  token cur_token_{ token_type::unknown, 0 };

  // used for informative error messages
  const char* expected_type_ = "";
  std::vector<std::string_view> term_toks_;
};

ast_node_ptr parse(std::string_view source, source_map* smap = nullptr);

}  // namespace mparse