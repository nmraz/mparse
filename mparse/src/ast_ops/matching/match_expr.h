#pragma once

#include "ast_ops/matching/util.h"
#include "mparse/ast/abs_node.h"
#include "mparse/ast/ast_node.h"
#include "mparse/ast/cast.h"
#include "mparse/ast/id_node.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/paren_node.h"
#include "mparse/ast/unary_node.h"
#include <type_traits>

namespace ast_ops::matching {

template<typename T>
constexpr bool is_match_expr = false;


/* ARITHMETIC MATCHERS */

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


template<typename Pred, typename Lhs, typename Rhs, bool Commute>
struct binary_op_pred_matcher {
  using match_type = mparse::binary_op_node;

  const Lhs lhs;
  const Rhs rhs;
};

template<typename Pred, typename Lhs, typename Rhs, bool Commute>
constexpr bool is_match_expr<binary_op_pred_matcher<Pred, Lhs, Rhs, Commute>> = true;


template<typename Node, typename Inner>
struct unary_matcher {
  static_assert(std::is_base_of_v<mparse::unary_node, Node>, "unary_node_matcher can only match descendants of unary_node");

  const Inner inner;
};

template<typename Node, typename Inner>
constexpr bool is_match_expr<unary_matcher<Node, Inner>> = true;


template<typename Pred, typename Inner>
struct unary_op_pred_matcher {
  using match_type = mparse::unary_op_node;

  const Inner inner;
};

template<typename Pred, typename Inner>
constexpr bool is_match_expr<unary_op_pred_matcher<Pred, Inner>> = true;


template<typename T, T Val>
struct type_eq_pred {
  constexpr bool operator()(T val) {
    return val == Val;
  }
};

template<mparse::binary_op_type Type, typename Lhs, typename Rhs, bool Commute = is_commutative(Type)>
using binary_op_matcher = binary_op_pred_matcher<type_eq_pred<mparse::binary_op_type, Type>, Lhs, Rhs, Commute>;

template<mparse::unary_op_type Type, typename Inner>
using unary_op_matcher = unary_op_pred_matcher<type_eq_pred<mparse::unary_op_type, Type>, Inner>;


template<
  typename Inner,
  typename = std::enable_if_t<is_match_expr<Inner>>
> constexpr unary_matcher<mparse::abs_node, Inner> abs(Inner inner) {
  return { inner };
}

template<
  typename Inner,
  typename = std::enable_if_t<is_match_expr<Inner>>
> constexpr unary_matcher<mparse::paren_node, Inner> paren(Inner inner) {
  return { inner };
}

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
> constexpr binary_op_matcher<mparse::binary_op_type::add, Lhs, Rhs, false> add_nocomm(Lhs lhs, Rhs rhs) {
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
> constexpr binary_op_matcher<mparse::binary_op_type::mult, Lhs, Rhs, false> mul_nocomm(Lhs lhs, Rhs rhs) {
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


/* COMBINATORS */

template<typename Matcher>
struct negation_matcher {
  const Matcher matcher;
};

template<typename Matcher>
constexpr bool is_match_expr<negation_matcher<Matcher>> = true;


template<typename First, typename Second>
struct conjunction_matcher {
  const First first;
  const Second second;
};

template<typename First, typename Second>
constexpr bool is_match_expr<conjunction_matcher<First, Second>> = true;


template<typename First, typename Second>
struct disjunction_matcher {
  const First first;
  const Second second;
};

template<typename First, typename Second>
constexpr bool is_match_expr<disjunction_matcher<First, Second>> = true;


template<
  typename Matcher,
  typename = std::enable_if_t<is_match_expr<Matcher>>
> constexpr negation_matcher<Matcher> match_not(Matcher matcher) {
  return { matcher };
}

template<
  typename First,
  typename Second,
  typename = std::enable_if_t<is_match_expr<First> && is_match_expr<Second>>
> constexpr conjunction_matcher<First, Second> match_and(First first, Second second) {
  return { first, second };
}

template<
  typename First,
  typename Second,
  typename = std::enable_if_t<is_match_expr<First> && is_match_expr<Second>>
> constexpr disjunction_matcher<First, Second> match_or(First first, Second second) {
  return { first, second };
}


inline namespace literals {

constexpr literal_matcher operator""_lit(long double val) {
  return { static_cast<double>(val) };
}

}  // namespace literals
}  // namespace ast_ops::matching