#pragma once

#include "mparse/ast/operator_nodes.h"
#include <type_traits>

namespace ast_ops::matching {

template<typename T>
constexpr bool is_match_expr = false;


struct literal_matcher {
  const double val;
};

template<>
constexpr bool is_match_expr<literal_matcher> = true;


template<std::size_t I, typename Node>
struct node_type_matcher {
  static_assert(std::is_base_of_v<mparse::ast_node, Node>, "node_type_matcher can only match descendants of ast_node");

  using match_type = Node;
  static constexpr std::size_t match_index = I;
};

template<std::size_t I>
using unique_symbol_matcher = node_type_matcher<I, mparse::ast_node>;

template<std::size_t I, typename Node>
constexpr bool is_match_expr<node_type_matcher<I, Node>> = true;


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


template<
  typename Inner,
  typename = std::enable_if_t<is_match_expr<Inner>>
> constexpr unary_op_matcher<mparse::unary_op_type::plus, Inner> operator+(Inner inner) {
  return { inner };
}

template<
  typename Inner,
  typename = std::enable_if_t<is_match_expr<Inner>>
> constexpr unary_op_matcher<mparse::unary_op_type::neg, Inner> operator-(Inner inner) {
  return { inner };
}

template<
  typename Lhs,
  typename Rhs,
  typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>
> constexpr binary_op_matcher<mparse::binary_op_type::add, Lhs, Rhs> operator+(Lhs lhs, Rhs rhs) {
  return { lhs, rhs };
}

template<
  typename Lhs,
  typename Rhs,
  typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>
> constexpr binary_op_matcher<mparse::binary_op_type::sub, Lhs, Rhs> operator-(Lhs lhs, Rhs rhs) {
  return { lhs, rhs };
}

template<
  typename Lhs,
  typename Rhs,
  typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>
> constexpr binary_op_matcher<mparse::binary_op_type::mult, Lhs, Rhs> operator*(Lhs lhs, Rhs rhs) {
  return { lhs, rhs };
}

template<
  typename Lhs,
  typename Rhs,
  typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>
> constexpr binary_op_matcher<mparse::binary_op_type::div, Lhs, Rhs> operator/(Lhs lhs, Rhs rhs) {
  return { lhs, rhs };
}

inline namespace literals {

constexpr literal_matcher operator""_lit(long double val) {
  return { static_cast<double>(val) };
}

}  // namespace literals
}  // namespace ast_ops::matching