#pragma once

#include "ast_ops/matching/match_results.h"
#include "ast_ops/matching/compare.h"
#include "ast_ops/matching/util.h"
#include "mparse/ast/abs_node.h"
#include "mparse/ast/ast_node.h"
#include "mparse/ast/paren_node.h"
#include "mparse/ast/unary_node.h"
#include <type_traits>

namespace ast_ops::matching {

template<typename T>
constexpr bool is_match_expr = false;


/* ARITHMETIC MATCHERS */

struct literal_expr {
  const double val;
};

template<>
constexpr bool is_match_expr<literal_expr> = true;


template<typename Node>
struct node_type_expr {
  static_assert(std::is_base_of_v<mparse::ast_node, Node>, "node_type_expr can only match descendants of ast_node")
};

template<typename Node>
constexpr bool is_match_expr<node_type_expr<Node>> = true;


template<typename Pred, typename Lhs, typename Rhs, bool Commute>
struct binary_op_pred_expr {
  const Lhs lhs;
  const Rhs rhs;
};

template<typename Pred, typename Lhs, typename Rhs, bool Commute>
constexpr bool is_match_expr<binary_op_pred_expr<Pred, Lhs, Rhs, Commute>> = true;


template<typename Node, typename Inner>
struct unary_expr {
  static_assert(std::is_base_of_v<mparse::unary_node, Node>, "unary_node_matcher can only match descendants of unary_node");
  const Inner inner;
};

template<typename Node, typename Inner>
constexpr bool is_match_expr<unary_expr<Node, Inner>> = true;


template<typename Pred, typename Inner>
struct unary_op_pred_expr {
  const Inner inner;
};

template<typename Pred, typename Inner>
constexpr bool is_match_expr<unary_op_pred_expr<Pred, Inner>> = true;


template<auto Val>
struct type_eq_pred {
  constexpr bool operator()(decltype(Val) val) {
    return val == Val;
  }
};

template<mparse::binary_op_type Type, typename Lhs, typename Rhs, bool Commute = is_commutative(Type)>
using binary_op_expr = binary_op_pred_expr<type_eq_pred<Type>, Lhs, Rhs, Commute>;

template<mparse::unary_op_type Type, typename Inner>
using unary_op_expr = unary_op_pred_expr<type_eq_pred<Type>, Inner>;


template<
  typename Inner,
  typename = std::enable_if_t<is_match_expr<Inner>>
> constexpr unary_expr<mparse::abs_node, Inner> abs(Inner inner) {
  return { inner };
}

template<
  typename Inner,
  typename = std::enable_if_t<is_match_expr<Inner>>
> constexpr unary_expr<mparse::paren_node, Inner> paren(Inner inner) {
  return { inner };
}

template<
  typename Inner,
  typename = std::enable_if_t<is_match_expr<Inner>>
> constexpr unary_op_expr<mparse::unary_op_type::plus, Inner> operator+(Inner inner) {
  return { inner };
}

template<
  typename Inner,
  typename = std::enable_if_t<is_match_expr<Inner>>
> constexpr unary_op_expr<mparse::unary_op_type::neg, Inner> operator-(Inner inner) {
  return { inner };
}

template<
  typename Lhs,
  typename Rhs,
  typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>
> constexpr binary_op_expr<mparse::binary_op_type::add, Lhs, Rhs> operator+(Lhs lhs, Rhs rhs) {
  return { lhs, rhs };
}

template<
  typename Lhs,
  typename Rhs,
  typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>
> constexpr binary_op_expr<mparse::binary_op_type::add, Lhs, Rhs, false> add_nocomm(Lhs lhs, Rhs rhs) {
  return { lhs, rhs };
}

template<
  typename Lhs,
  typename Rhs,
  typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>
> constexpr binary_op_expr<mparse::binary_op_type::sub, Lhs, Rhs> operator-(Lhs lhs, Rhs rhs) {
  return { lhs, rhs };
}

template<
  typename Lhs,
  typename Rhs,
  typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>
> constexpr binary_op_expr<mparse::binary_op_type::mult, Lhs, Rhs> operator*(Lhs lhs, Rhs rhs) {
  return { lhs, rhs };
}

template<
  typename Lhs,
  typename Rhs,
  typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>
> constexpr binary_op_expr<mparse::binary_op_type::mult, Lhs, Rhs, false> mul_nocomm(Lhs lhs, Rhs rhs) {
  return { lhs, rhs };
}

template<
  typename Lhs,
  typename Rhs,
  typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>
> constexpr binary_op_expr<mparse::binary_op_type::div, Lhs, Rhs> operator/(Lhs lhs, Rhs rhs) {
  return { lhs, rhs };
}

template<
  typename Lhs,
  typename Rhs,
  typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>
> constexpr binary_op_expr<mparse::binary_op_type::pow, Lhs, Rhs> pow(Lhs lhs, Rhs rhs) {
  return { lhs, rhs };
}


/* COMBINATORS */

template<typename Expr>
struct negation_expr {
  const Expr expr;
};

template<typename Expr>
constexpr bool is_match_expr<negation_expr<Expr>> = true;


template<typename First, typename Second>
struct conjunction_expr {
  const First first;
  const Second second;
};

template<typename First, typename Second>
constexpr bool is_match_expr<conjunction_expr<First, Second>> = true;


template<typename First, typename Second>
struct disjunction_expr {
  const First first;
  const Second second;
};

template<typename First, typename Second>
constexpr bool is_match_expr<disjunction_expr<First, Second>> = true;


template<
  typename Expr,
  typename = std::enable_if_t<is_match_expr<Expr>>
> constexpr negation_expr<Expr> match_not(Expr expr) {
  return { expr };
}

template<
  typename First,
  typename Second,
  typename = std::enable_if_t<is_match_expr<First> && is_match_expr<Second>>
> constexpr conjunction_expr<First, Second> match_and(First first, Second second) {
  return { first, second };
}

template<
  typename First,
  typename Second,
  typename = std::enable_if_t<is_match_expr<First> && is_match_expr<Second>>
> constexpr disjunction_expr<First, Second> match_or(First first, Second second) {
  return { first, second };
}


/* CAPTURING EXPRESSIONS */

template<typename Tag, typename Expr>
struct capture_expr_impl {
  const Expr expr;
};

template<typename Tag, typename Expr>
constexpr bool is_match_expr<capture_expr_impl<Tag, Expr>> = true;


template<int N>
struct capture_expr_tag {};

template<int N, typename Res>
decltype(auto) get_capture(Res&& results) {
  return get_result<capture_expr_tag<N>>(std::forward<Res>(results));
}

template<int N, typename Expr>
using capture_expr = capture_expr_impl<capture_expr_tag<N>, Expr>;


template<int N>
struct constant_tag {};

template<int N, typename Res>
decltype(auto) get_constant(Res&& results) {
  return get_result<constant_tag<N>>(std::forward<Res>(results));
}

template<int N>
using constant_expr = capture_expr_impl<constant_tag<N>, node_type_expr<mparse::literal_node>>;


template<char C>
struct subexpr_expr_tag {};

template<char C, typename Res>
decltype(auto) get_subexpr(Res&& results) {
  return get_result<subexpr_expr_tag<C>>(std::forward<Res>(results));
}

template<char C, typename Comp = commutative_expr_comparer>
struct subexpr_expr {
  const Comp comp{};
};

template<char C, typename Comp>
constexpr bool is_match_expr<subexpr_expr<C, Comp>> = true;


inline namespace literals {

constexpr literal_expr operator""_lit(long double val) {
  return { static_cast<double>(val) };
}

constexpr constant_expr<1> c1{};
constexpr constant_expr<2> c2{};
constexpr constant_expr<3> c3{};

constexpr subexpr_expr<'x'> x{};
constexpr subexpr_expr<'y'> y{};
constexpr subexpr_expr<'z'> z{};

}  // namespace literals
}  // namespace ast_ops::matching