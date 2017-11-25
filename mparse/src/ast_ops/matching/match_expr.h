#pragma once

#include "mparse/ast/operator_nodes.h"
#include <type_traits>

namespace ast_ops::matching {

template<typename T>
constexpr bool is_match_expr = false;

template<std::size_t I>
struct unique_symbol_matcher {};

template<std::size_t I>
constexpr bool is_match_expr<unique_symbol_matcher<I>> = true;


struct literal_matcher {
  const double val;
};

template<>
constexpr bool is_match_expr<literal_matcher> = true;


template<std::size_t I, typename Node>
struct node_type_matcher {
  static_assert(std::is_base_of_v<mparse::ast_node, Node>, "node_type_matcher can only match descendants of ast_node");
};

template<std::size_t I, typename Node>
constexpr bool is_match_expr<node_type_matcher<T, Node>> = true;


template<mparse::binary_op_type Type, typename Lhs, typename Rhs>
struct binary_op_matcher {
  const Lhs lhs;
  const Rhs rhs;
};

template<mparse::binary_op_type Type, typename Lhs, typename Rhs>
constexpr bool is_match_expr<binary_op_matcher<Type, Lhs, Rhs>> = true;


template<mparse::unary_op_type Type, typename Inner>
struct unary_op_matcher {
  const Inner inner;
};

template<mparse::unary_op_type Type, typename Inner>
constexpr bool is_match_expr<unary_op_matcher<Type, Inner>> = true;

}  // namespace ast_ops::matching