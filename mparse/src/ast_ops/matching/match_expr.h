#pragma once

#include "ast_ops/matching/match_results.h"
#include "ast_ops/matching/compare.h"
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

  template<typename Ctx>
  bool matches(const mparse::ast_node_ptr& node, Ctx& ctx) const {
    if (auto* lit_node = mparse::ast_node_cast<const mparse::literal_node>(node.get())) {
      return lit_node->val() == val;
    }
    return false;
  }

  const double val;
};

template<>
constexpr bool is_match_expr<literal_matcher> = true;


template<typename Node>
struct node_type_matcher {
  static_assert(std::is_base_of_v<mparse::ast_node, Node>, "node_type_matcher can only match descendants of ast_node");
  using match_type = Node;

  template<typename Ctx>
  bool matches(const mparse::ast_node_ptr& node, Ctx& ctx) const {
    return mparse::ast_node_cast<const Node>(node.get()) != nullptr;
  }
};

template<typename Node>
constexpr bool is_match_expr<node_type_matcher<Node>> = true;


template<typename Pred, typename Lhs, typename Rhs, bool Commute>
struct binary_op_pred_matcher {
  using match_type = mparse::binary_op_node;
  using captures = caplist_cat<get_captures_t<Lhs>, get_captures_t<Rhs>>;

  template<typename Ctx>
  bool matches(const mparse::ast_node_ptr& node, Ctx& ctx) const {
    if (auto* bin_node = mparse::ast_node_cast<mparse::binary_op_node>(node.get())) {
      if (!Pred{}(bin_node->type())) {
        return false;
      }

      auto node_lhs = bin_node->ref_lhs();
      auto node_rhs = bin_node->ref_rhs();

      if (lhs.matches(node_lhs, ctx) && rhs.matches(node_rhs, ctx)) {
        return true;
      } else if (Commute && rhs.matches(node_lhs, ctx) && lhs.matches(node_rhs, ctx)) {
        return true;
      }
      return false;
    }
  }

  const Lhs lhs;
  const Rhs rhs;
};

template<typename Pred, typename Lhs, typename Rhs, bool Commute>
constexpr bool is_match_expr<binary_op_pred_matcher<Pred, Lhs, Rhs, Commute>> = true;


template<typename Node, typename Inner>
struct unary_matcher {
  static_assert(std::is_base_of_v<mparse::unary_node, Node>, "unary_node_matcher can only match descendants of unary_node");

  using match_type = mparse::unary_node;
  using captures = get_captures_t<Inner>;

  template<typename Ctx>
  bool matches(const mparse::ast_node_ptr& node, Ctx& ctx) const {
    if (auto* un_node = mparse::ast_node_cast<mparse::unary_node>(node.get())) {
      return inner.matches(un_node->ref_child(), ctx);
    }
    return false;
  }

  const Inner inner;
};

template<typename Node, typename Inner>
constexpr bool is_match_expr<unary_matcher<Node, Inner>> = true;


template<typename Pred, typename Inner>
struct unary_op_pred_matcher {
  using match_type = mparse::unary_op_node;
  using captures = get_captures_t<Inner>;

  template<typename Ctx>
  bool matches(const mparse::ast_node_ptr& node, Ctx& ctx) const {
    if (auto* un_op_node = mparse::ast_node_cast<mparse::unary_op_node>(node.get())) {
      return Pred{}(un_op_node->type()) && inner.matches(un_op_node->ref_child(), ctx);
    }
  }

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
  using captures = get_captures_t<Matcher>;

  template<typename Ctx>
  bool matches(const mparse::ast_node_ptr& node, Ctx& ctx) const {
    return !matcher.matches(node, ctx);
  }

  const Matcher matcher;
};

template<typename Matcher>
constexpr bool is_match_expr<negation_matcher<Matcher>> = true;


template<typename First, typename Second>
struct conjunction_matcher {
  using captures = caplist_cat<get_captures_t<First>, get_captures_t<Second>>;

  template<typename Ctx>
  bool matches(const mparse::ast_node_ptr& node, Ctx& ctx) const {
    return first.matches(node, ctx) && second.matches(node, ctx);
  }

  const First first;
  const Second second;
};

template<typename First, typename Second>
constexpr bool is_match_expr<conjunction_matcher<First, Second>> = true;


template<typename First, typename Second>
struct disjunction_matcher {
  using captures = caplist_cat<get_captures_t<First>, get_captures_t<Second>>;

  template<typename Ctx>
  bool matches(const mparse::ast_node_ptr& node, Ctx& ctx) const {
    return first.matches(node, ctx) || second.matches(node, ctx);
  }

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


/* CAPTURING MATCHERS */

template<typename Matcher, typename = void>
struct get_match_type {
  using type = mparse::ast_node;
};

template<typename Matcher>
struct get_match_type<Matcher, std::void_t<typename Matcher::match_type>> {
  using type = typename Matcher::match_type;
};

template<typename Matcher>
using get_match_type_t = typename get_match_type<Matcher>::type;

template<typename Tag, typename Matcher>
struct capture_matcher_impl {
  using captures = caplist_append<
    get_captures_t<Matcher>,
    capture<Tag, mparse::node_ptr<get_match_type_t<Matcher>>>
  >;

  template<typename Ctx>
  bool matches(const mparse::ast_node_ptr& node, Ctx& ctx) {
    if (matcher.matches(node, ctx)) {
      get_result<Tag>(ctx) = mparse::static_ast_node_ptr_cast<get_match_type_t<Matcher>>(node);
      return true;
    }
    return false;
  }

  const Matcher matcher;
};

template<typename Tag, typename Matcher>
constexpr bool is_match_expr<capture_matcher_impl<Tag, Matcher>> = true;


template<int N>
struct capture_matcher_tag {};

template<int N, typename Res>
decltype(auto) get_capture(Res&& results) {
  return get_result<capture_matcher_tag<N>>(std::forward<Res>(results));
}

template<int N, typename Matcher>
using capture_matcher = capture_matcher_impl<capture_matcher_tag<N>, Matcher>;


template<int N>
struct constant_tag {};

template<int N, typename Res>
decltype(auto) get_constant(Res&& results) {
  return get_result<constant_tag<N>>(std::forward<Res>(results));
}

template<int N>
using constant_matcher = capture_matcher_impl<constant_tag<N>, node_type_matcher<mparse::literal_node>>;


template<char C>
struct subexpr_matcher_tag {};

template<char C, typename Res>
decltype(auto) get_subexpr(Res&& results) {
  return get_result<subexpr_matcher_tag<C>>(std::forward<Res>(results));
}

template<char C, typename Comp = commutative_expr_comparer>
struct subexpr_matcher {
  using captures = caplist<capture<subexpr_matcher_tag<C>, mparse::ast_node_ptr>>;

  template<typename Ctx>
  bool matches(const mparse::ast_node_ptr& node, Ctx& ctx) {
    mparse::ast_node_ptr& saved = get_subexpr<C>(ctx);
    if (!saved) {
      saved = node;
      return true;
    }
    return compare_exprs(saved.get(), node.get(), comp);
  }

  const Comp comp{};
};

template<char C, typename Comp>
constexpr bool is_match_expr<subexpr_matcher<C, Comp>> = true;

}  // namespace ast_ops::matching