#pragma once

#include "mparse/ast/ast_node.h"
#include "mparse/lex.h"
#include "mparse/source_map.h"
#include "mparse/source_stream.h"
#include <memory>

namespace mparse {

class parser {
public:
  parser(source_stream& stream, source_map* smap = nullptr);
  ~parser();

  void begin_parse();
  void end_parse();

  ast_node_ptr parse_root();

  ast_node_ptr parse_add();
  ast_node_ptr parse_mult();
  ast_node_ptr parse_unary();
  ast_node_ptr parse_pow();
  ast_node_ptr parse_atom();

  void get_next_token();
  token cur_token() const;

private:
  struct parser_impl;
  std::unique_ptr<parser_impl> impl_;
};

ast_node_ptr parse(std::string_view source, source_map* smap = nullptr);

} // namespace mparse