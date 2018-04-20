#include "parser.h"

#include "mparse/ast/abs_node.h"
#include "mparse/ast/id_node.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/paren_node.h"
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
  case token_type::ident:
    return "identifier";
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
  return { tok.loc, tok.loc + std::max(tok.val.size(), std::size_t{1}) };
}

}  // namespace


parser::parser(source_stream& stream, source_map* smap)
  : stream_(stream)
  , smap_(smap) {
}


void parser::begin_parse() {
  get_next_token();
}

void parser::end_parse() {
  if (cur_token_.type != token_type::eof) {
    error();
  }
}


ast_node_ptr parser::parse_root() {
  begin_parse();
  ast_node_ptr expr = parse_add();
  end_parse();
  return expr;
}


ast_node_ptr parser::parse_add() {
  static constexpr delim_val_mapping<binary_op_type> ops[] = {
    { "+"sv, binary_op_type::add },
    { "-"sv, binary_op_type::sub }
  };

  const binary_op_type* op = nullptr;
  ast_node_ptr node = parse_mult();

  while ((op = find_delim_val(ops, cur_token_)) != nullptr) {
    source_range op_loc = get_loc(cur_token_);
  
    get_next_token();
    auto add_node = make_ast_node<binary_op_node>(*op, std::move(node), parse_mult());

    save_bin_locs(add_node.get(), op_loc);
    node = std::move(add_node);
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

  while ((op = find_delim_val(ops, cur_token_)) != nullptr) {
    source_range op_loc = get_loc(cur_token_);

    get_next_token();
    auto mul_node = make_ast_node<binary_op_node>(*op, std::move(node), parse_unary());

    save_bin_locs(mul_node.get(), op_loc);
    node = std::move(mul_node);
  }

  return node;
}

ast_node_ptr parser::parse_unary() {
  static constexpr delim_val_mapping<unary_op_type> ops[] = {
    { "+"sv, unary_op_type::plus },
    { "-"sv, unary_op_type::neg }
  };

  const unary_op_type* op = find_delim_val(ops, cur_token_);
  if (op) {
    source_range op_loc = get_loc(cur_token_);

    get_next_token();
    auto node = make_ast_node<unary_op_node>(*op, parse_unary());
    
    if (smap_) {
      smap_->set_locs(node.get(), {
        source_range::merge(op_loc, smap_->find_primary_loc(node->child())),  // full range
        op_loc  // operator
      });
    }

    return node;
  }

  return parse_pow();
}

ast_node_ptr parser::parse_pow() {
  ast_node_ptr node = parse_atom();

  if (has_delim("^"sv)) {
    source_range op_loc = get_loc(cur_token_);

    get_next_token();
    auto pow_node = make_ast_node<binary_op_node>(binary_op_type::pow, std::move(node), parse_unary());

    save_bin_locs(pow_node.get(), op_loc);
    return pow_node;
  }

  return node;
}


ast_node_ptr parser::parse_atom() {
  expected_type_ = "an expression";  // at this point, we want an expression

  ast_node_ptr ret = nullptr;

  if (cur_token_.type == token_type::literal) {
    ret = consume_literal();
  } else if (cur_token_.type == token_type::ident) {
    ret = consume_ident();
  } else if (has_delim("("sv)) {
    ret = consume_paren_like<paren_node>(")"sv, "parentheses");
  } else if (has_delim("|"sv)) {
    ret = consume_paren_like<abs_node>("|"sv, "absolute value bars");
  } else {
    error();
  }

  expected_type_ = "an operator";  // we need an operator to follow the expression
  return ret;
}

ast_node_ptr parser::consume_literal() {
  auto node = make_ast_node<literal_node>(std::strtod(cur_token_.val.data(), nullptr));
  if (smap_) {
    smap_->set_locs(node.get(), { get_loc(cur_token_) });
  }

  get_next_token();
  return node;
}

ast_node_ptr parser::consume_ident() {
  auto node = make_ast_node<id_node>(std::string(cur_token_.val));
  if (smap_) {
    smap_->set_locs(node.get(), { get_loc(cur_token_) });
  }

  get_next_token();
  return node;
}

template<typename T>
ast_node_ptr parser::consume_paren_like(std::string_view term_tok, const char* friendly_name) {
  source_range open_loc = get_loc(cur_token_);
  push_term_tok(term_tok);

  get_next_token();
  ast_node_ptr inner_expr = parse_add();

  if (!has_delim(term_tok)) {
    if (has_term_tok()) {
      throw syntax_error("Unbalanced "s + friendly_name, { open_loc, get_loc(cur_token_) });
    }
    error();
  }
  pop_term_tok();
  source_range close_loc = get_loc(cur_token_);

  get_next_token();
  auto node = make_ast_node<T>(std::move(inner_expr));

  if (smap_) {
    smap_->set_locs(node.get(), { source_range::merge(open_loc, close_loc) });
  }
  return node;
}


void parser::get_next_token() {
  cur_token_ = get_token(stream_);
}


// PRIVATE

void parser::push_term_tok(std::string_view term_tok) {
  term_toks_.push_back(term_tok);
}

void parser::pop_term_tok() {
  term_toks_.pop_back();
}


bool parser::has_term_tok() const {
  if (cur_token_.type == token_type::eof) {
    return true;
  }

  if (cur_token_.type != token_type::delim) {
    return false;
  }

  return std::find(term_toks_.begin(), term_toks_.end(), cur_token_.val) != term_toks_.end();
}

bool parser::has_delim(std::string_view val) const {
  return cur_token_.type == token_type::delim && cur_token_.val == val;
}

void parser::error() const {
  std::string msg = "Unexpected "s + token_type_name(cur_token_.type) + ": expected " + expected_type_;

  throw syntax_error(
    msg,
    { get_loc(cur_token_) }
  );
}

void parser::save_bin_locs(const binary_op_node* node, source_range op_loc) {
  if (smap_) {
    smap_->set_locs(node, {
      source_range::merge(smap_->find_primary_loc(node->lhs()), smap_->find_primary_loc(node->rhs())),  // full range
      op_loc  // operator
    });
  }
}


ast_node_ptr parse(std::string_view source, source_map* smap) {
  source_stream stream(source);
  parser p(stream, smap);
  return p.parse_root();
}

}  // namespace mparse