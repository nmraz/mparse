#pragma once

#include "ast_ops/matching/compare.h"
#include "ast_ops/matching/util.h"
#include "mparse/ast/abs_node.h"
#include "mparse/ast/ast_node.h"
#include "mparse/ast/paren_node.h"
#include "mparse/ast/unary_node.h"
#include <string_view>
#include <tuple>
#include <type_traits>

namespace ast_ops::matching {

template <typename T>
constexpr bool is_match_expr = false;


/* CUSTOM EXPRESSIONS */

template <typename Node, typename Pred, typename... Caps>
struct custom_matcher_expr {
  static_assert(std::is_base_of_v<mparse::ast_node, Node>,
                "custom_matcher_expr can only match descendants of ast_node");

  const Pred pred;
};

template <typename Node, typename Pred, typename... Caps>
constexpr bool is_match_expr<custom_matcher_expr<Node, Pred, Caps...>> = true;


struct always_true_pred {
  template <typename T>
  constexpr bool operator()(const T&) {
    return true;
  }
};

template <typename Node>
using node_type_expr = custom_matcher_expr<Node, always_true_pred>;


template <typename F, typename... Tags>
struct custom_builder_expr {
  const F func;
};

template <typename F, typename... Tags>
constexpr bool is_match_expr<custom_builder_expr<F, Tags...>> = true;


/* ARITHMETIC EXPRESSIONS */

struct literal_expr {
  const double val;
};

template <>
constexpr bool is_match_expr<literal_expr> = true;


struct id_expr {
  const std::string_view name;
};

template <>
constexpr bool is_match_expr<id_expr> = true;


template <typename... Args>
struct func_expr {
  const std::string_view name;
  const std::tuple<Args...> args;
};

template <typename... Args>
constexpr bool is_match_expr<func_expr<Args...>> = true;


template <typename Pred, typename Lhs, typename Rhs, bool Commute>
struct binary_op_pred_expr {
  const Lhs lhs;
  const Rhs rhs;
};

template <typename Pred, typename Lhs, typename Rhs, bool Commute>
constexpr bool is_match_expr<binary_op_pred_expr<Pred, Lhs, Rhs, Commute>> =
    true;


template <typename Node, typename Inner>
struct unary_expr {
  static_assert(std::is_base_of_v<mparse::unary_node, Node>,
                "unary_node_matcher can only match descendants of unary_node");
  const Inner inner;
};

template <typename Node, typename Inner>
constexpr bool is_match_expr<unary_expr<Node, Inner>> = true;


template <typename Pred, typename Inner>
struct unary_op_pred_expr {
  const Inner inner;
};

template <typename Pred, typename Inner>
constexpr bool is_match_expr<unary_op_pred_expr<Pred, Inner>> = true;


// TODO: use template<auto> when MSVC bug is fixed
// See
// https://developercommunity.visualstudio.com/content/problem/337503/c2664-when-using-auto-template-parameter-with-enum.html

template <mparse::binary_op_type Val>
struct bin_type_eq_pred {
  constexpr bool operator()(mparse::binary_op_type val) { return val == Val; }
};

template <mparse::unary_op_type Val>
struct un_type_eq_pred {
  constexpr bool operator()(mparse::unary_op_type val) { return val == Val; }
};

template <mparse::binary_op_type Type, typename Lhs, typename Rhs,
          bool Commute = is_commutative(Type)>
using binary_op_expr =
    binary_op_pred_expr<bin_type_eq_pred<Type>, Lhs, Rhs, Commute>;

template <mparse::unary_op_type Type, typename Inner>
using unary_op_expr = unary_op_pred_expr<un_type_eq_pred<Type>, Inner>;


template <typename Inner, typename = std::enable_if_t<is_match_expr<Inner>>>
constexpr unary_expr<mparse::abs_node, Inner> abs(Inner inner) {
  return {inner};
}

template <typename Inner, typename = std::enable_if_t<is_match_expr<Inner>>>
constexpr unary_expr<mparse::paren_node, Inner> paren(Inner inner) {
  return {inner};
}

template <typename Inner, typename = std::enable_if_t<is_match_expr<Inner>>>
constexpr unary_op_expr<mparse::unary_op_type::plus, Inner> operator+(
    Inner inner) {
  return {inner};
}

template <typename Inner, typename = std::enable_if_t<is_match_expr<Inner>>>
constexpr unary_op_expr<mparse::unary_op_type::neg, Inner> operator-(
    Inner inner) {
  return {inner};
}

template <typename Lhs, typename Rhs,
          typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>>
constexpr binary_op_expr<mparse::binary_op_type::add, Lhs, Rhs> operator+(
    Lhs lhs, Rhs rhs) {
  return {lhs, rhs};
}

template <typename Lhs, typename Rhs,
          typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>>
constexpr binary_op_expr<mparse::binary_op_type::add, Lhs, Rhs, false>
add_nocomm(Lhs lhs, Rhs rhs) {
  return {lhs, rhs};
}

template <typename Lhs, typename Rhs,
          typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>>
constexpr binary_op_expr<mparse::binary_op_type::sub, Lhs, Rhs> operator-(
    Lhs lhs, Rhs rhs) {
  return {lhs, rhs};
}

template <typename Lhs, typename Rhs,
          typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>>
constexpr binary_op_expr<mparse::binary_op_type::mult, Lhs, Rhs> operator*(
    Lhs lhs, Rhs rhs) {
  return {lhs, rhs};
}

template <typename Lhs, typename Rhs,
          typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>>
constexpr binary_op_expr<mparse::binary_op_type::mult, Lhs, Rhs, false>
mul_nocomm(Lhs lhs, Rhs rhs) {
  return {lhs, rhs};
}

template <typename Lhs, typename Rhs,
          typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>>
constexpr binary_op_expr<mparse::binary_op_type::div, Lhs, Rhs> operator/(
    Lhs lhs, Rhs rhs) {
  return {lhs, rhs};
}

template <typename Lhs, typename Rhs,
          typename = std::enable_if_t<is_match_expr<Lhs> && is_match_expr<Rhs>>>
constexpr binary_op_expr<mparse::binary_op_type::pow, Lhs, Rhs> pow(Lhs lhs,
                                                                    Rhs rhs) {
  return {lhs, rhs};
}


/* COMBINATORS */

template <typename Expr>
struct negation_expr {
  const Expr expr;
};

template <typename Expr>
constexpr bool is_match_expr<negation_expr<Expr>> = true;


template <typename First, typename Second>
struct conjunction_expr {
  const First first;
  const Second second;
};

template <typename First, typename Second>
constexpr bool is_match_expr<conjunction_expr<First, Second>> = true;


template <typename First, typename Second>
struct disjunction_expr {
  const First first;
  const Second second;
};

template <typename First, typename Second>
constexpr bool is_match_expr<disjunction_expr<First, Second>> = true;


template <typename Expr, typename = std::enable_if_t<is_match_expr<Expr>>>
constexpr negation_expr<Expr> match_not(Expr expr) {
  return {expr};
}

template <
    typename First, typename Second,
    typename = std::enable_if_t<is_match_expr<First> && is_match_expr<Second>>>
constexpr conjunction_expr<First, Second> match_and(First first,
                                                    Second second) {
  return {first, second};
}

template <
    typename First, typename Second,
    typename = std::enable_if_t<is_match_expr<First> && is_match_expr<Second>>>
constexpr disjunction_expr<First, Second> match_or(First first, Second second) {
  return {first, second};
}


/* CAPTURING EXPRESSIONS */

template <typename Tag, typename Expr>
struct capture_expr_impl {
  const Expr expr{};
};

template <typename Tag, typename Expr>
constexpr bool is_match_expr<capture_expr_impl<Tag, Expr>> = true;


template <int N>
struct capture_expr_tag {};

template <int N, typename Expr>
using capture_expr = capture_expr_impl<capture_expr_tag<N>, Expr>;


struct retrieve_capture_dummy_expr {};

template <>
constexpr bool is_match_expr<retrieve_capture_dummy_expr> = true;

template <int N>
using retrieve_capture_expr =
    capture_expr_impl<capture_expr_tag<N>, retrieve_capture_dummy_expr>;


template <int N>
struct constant_expr_tag {};

template <int N>
using constant_expr = capture_expr_impl<constant_expr_tag<N>,
                                        node_type_expr<mparse::literal_node>>;


template <char C>
struct subexpr_expr_tag {};

template <char C, typename Comp = commutative_expr_comparer>
struct subexpr_expr {
  const Comp comp{};
};

template <char C, typename Comp>
constexpr bool is_match_expr<subexpr_expr<C, Comp>> = true;


inline namespace literals {
template <typename Node, typename Pred, typename... Caps>
constexpr custom_matcher_expr<Node, std::decay_t<Pred>, Caps...> match_custom(
    Pred&& pred, Caps...) {
  return {std::forward<Pred>(pred)};
}

template <typename F, typename... Tags>
constexpr custom_builder_expr<std::decay_t<F>, Tags...> build_custom(F&& func, Tags...) {
  return {std::forward<F>(func)};
}


constexpr id_expr id(std::string_view name) {
  return {name};
}

template <typename... Args,
          typename = std::enable_if_t<(... && is_match_expr<Args>)>>
constexpr func_expr<Args...> func(std::string_view name, Args... args) {
  return {name, {args...}};
}


constexpr literal_expr operator""_lit(long double val) {
  return {static_cast<double>(val)};
}

constexpr literal_expr operator""_lit(unsigned long long val) {
  return {static_cast<double>(val)};
}


template <int N, typename Expr,
          typename = std::enable_if_t<is_match_expr<Expr>>>
constexpr capture_expr<N, Expr> capture_as(Expr expr) {
  return {expr};
}

template <int N>
constexpr retrieve_capture_expr<N> cap{};


constexpr constant_expr<1> c1{};
constexpr constant_expr<2> c2{};
constexpr constant_expr<3> c3{};

constexpr subexpr_expr<'x'> x{};
constexpr subexpr_expr<'y'> y{};
constexpr subexpr_expr<'z'> z{};

} // namespace literals
} // namespace ast_ops::matching