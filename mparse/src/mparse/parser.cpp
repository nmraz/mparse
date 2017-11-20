#include "parser.h"

#include "mparse/ast/paren_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/literal_node.h"
#include "mparse/error.h"
#include <algorithm>
#include <array>
#include <string>
#include <utility>

using namespace std::literals;

namespace mparse {
namespace {

template<typename T>
struct delim_val_mapping {
  std::string_view tok;
  T val;
};

template<typename T, std::size_t N>
const T* find_delim_val(const delim_val_mapping<T>(&mapping)[N], const token& tok) {
  if (tok.type != token_type::delim) {
    return nullptr;
  }

  auto it = std::find_if(
    std::begin(mapping),
    std::end(mapping),
    [&tok] (const delim_val_mapping<T>& entry) {
      return entry.tok == tok.val;
    }
  );
  return it == std::end(mapping) ? nullptr : &it->val;
}


const char* token_type_name(token_type type) {
  switch (type) {
  case token_type::literal:
    return "number";
  case token_type::delim:
    return "delimitor";
  case token_type::eof:
    return "end of input";
  case token_type::unknown:
  default:
    return "token";
  }
}

source_range get_loc(const token& tok) {
  return { tok.loc, tok.loc + tok.val.size() };
}

}  // namespace


parser::parser(source_stream stream)
  : stream_(stream) {
}

ast_node_ptr parser::parse_root() {
  begin_parse();
  ast_node_ptr expr = parse_add();
  if (last_token_.type != token_type::eof) {
    error();
  }

  return expr;
}


ast_node_ptr parser::parse_add() {
  static constexpr delim_val_mapping<binary_op_type> ops[] = {
    { "+"sv, binary_op_type::add },
    { "-"sv, binary_op_type::sub }
  };

  const binary_op_type* op = nullptr;
  ast_node_ptr node = parse_mult();

  while ((op = find_delim_val(ops, last_token_)) != nullptr) {
    std::size_t op_loc = last_token_.loc;
  
    get_next_token();
    ast_node_ptr rhs = parse_mult();
    
    node = make_ast_node<binary_op_node>(*op, std::move(node), std::move(rhs));
    // TODO: set source info
  }

  return node;
}

ast_node_ptr parser::parse_mult() {
  static constexpr delim_val_mapping<binary_op_type> ops[] = {
    { "*"sv, binary_op_type::mult },
    { "/"sv, binary_op_type::div }
  };

  const binary_op_type* op = nullptr;
  ast_node_ptr node = parse_unary();

  while ((op = find_delim_val(ops, last_token_)) != nullptr) {
    std::size_t op_loc = last_token_.loc;

    get_next_token();
    ast_node_ptr rhs = parse_unary();

    node = make_ast_node<binary_op_node>(*op, std::move(node), std::move(rhs));
    // TODO: set source info
  }

  return node;
}

ast_node_ptr parser::parse_unary() {
  static constexpr delim_val_mapping<unary_op_type> ops[] = {
    { "+"sv, unary_op_type::plus },
    { "-"sv, unary_op_type::neg }
  };

  const unary_op_type* op = find_delim_val(ops, last_token_);
  if (op) {
    std::size_t op_loc = last_token_.loc;

    get_next_token();
    return make_ast_node<unary_op_node>(*op, parse_unary());
    // TODO: set source info
  }

  return parse_pow();
}

ast_node_ptr parser::parse_pow() {
  ast_node_ptr node = parse_atom();

  if (has_delim("^"sv)) {
    std::size_t op_loc = last_token_.loc;

    get_next_token();
    return make_ast_node<binary_op_node>(binary_op_type::pow, std::move(node), parse_unary());
    // TODO: set source info
  }

  return node;
}


ast_node_ptr parser::parse_atom() {
  expected_type_ = "an expression";  // at this point, we want an expression

  ast_node_ptr ret = nullptr;

  if (last_token_.type == token_type::literal) {
    ret = consume_literal();
  } else if (has_delim("("sv)) {
    ret = consume_paren();
  } else {
    error();
  }

  expected_type_ = "an operator";  // we need an operator to follow the expression
  return ret;
}

ast_node_ptr parser::consume_literal() {
  token tok = last_token_;
  get_next_token();
  return make_ast_node<literal_node>(std::strtod(tok.val.data(), nullptr));
  // TODO: set source info
}

ast_node_ptr parser::consume_paren() {
  std::size_t open_loc = last_token_.loc;

  get_next_token();
  ast_node_ptr inner_expr = parse_add();

  if (!has_delim(")"sv)) {
    if (last_token_.type == token_type::eof) {
      throw syntax_error("Unbalanced parentheses", source_range(open_loc));
    }
    error();
  }
  std::size_t close_loc = last_token_.loc + last_token_.val.size();

  get_next_token();
  return make_ast_node<paren_node>(std::move(inner_expr));
  // TODO: set source info
}


// PRIVATE

void parser::get_next_token() {
  last_token_ = get_token(stream_);
}

bool parser::has_delim(std::string_view val) const {
  return last_token_.type == token_type::delim && last_token_.val == val;
}

void parser::error() const {
  if (last_token_.type == token_type::eof) {
    // custom message/location for end of input
    throw syntax_error(
      "Expected "s + expected_type_,
      source_range(last_token_.loc, last_token_.loc + 1)
    );
  }

  std::string msg = "Unexpected "s + token_type_name(last_token_.type)
    + " '" + std::string(last_token_.val) + "': expected " + expected_type_;

  throw syntax_error(
    msg,
    get_loc(last_token_)
  );
}


ast_node_ptr parse(std::string_view source) {
  source_stream stream(source);
  parser p(stream);
  return p.parse_root();
}

}  // namespace mparse