#pragma once

#include "ast_ops/matching/util.h"
#include "mparse/ast/operator_nodes.h"
#include <type_traits>

namespace ast_ops::matching {

template<typename T>
constexpr bool is_match_expr = false;


struct literal_matcher {
  using match_type = mparse::literal_node;

  const double val;
};

template<>
constexpr bool is_match_expr<literal_matcher> = true;


template<typename Node>
struct node_type_matcher {
  static_assert(std::is_base_of_v<mparse::ast_node, Node>, "node_type_matcher can only match descendants of ast_node");
  using match_type = Node;
};

template<typename Node>
constexpr bool is_match_expr<node_type_matcher<Node>> = true;


template<mparse::binary_op_type Type, typename Lhs, typename Rhs, bool Commute = is_commutative(Type)>
struct binary_op_matcher {
  using match_type = mparse::binary_op_node;

  const Lhs lhs;
  const Rhs rhs;
};

template<mparse::binary_op_type Type, typename Lhs, typename Rhs, bool Commute>
constexpr bool is_match_expr<binary_op_matcher<Type, Lhs, Rhs, Commute>> = true;


template<typename Node, typename Inner>
struct unary_matcher {
  static_assert(std::is_base_of_v<mparse::unary_node, Node>, "unary_node_matcher can only match descendants of unary_node");

  const Inner inner;
};

template<typename Node, typename Inner>
constexpr bool is_match_expr<unary_matcher<Node, Inner>> = true;


template<mparse::unary_op_type Type, typename Inner>
struct unary_op_matcher {
  using match_type = mparse::unary_op_node;

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

template<
  typename Lhs,
  typename Rhs,
  typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>
> constexpr binary_op_matcher<mparse::binary_op_type::pow, Lhs, Rhs> pow(Lhs lhs, Rhs rhs) {
  return { lhs, rhs };
}


inline namespace literals {

constexpr literal_matcher operator""_lit(long double val) {
  return { static_cast<double>(val) };
}

}  // namespace literals
}  // namespace ast_ops::matching