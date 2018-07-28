#pragma once

#include "ast_ops/matching/compare.h"
#include "ast_ops/matching/match_expr.h"
#include "ast_ops/matching/match_results.h"
#include "mparse/ast/ast_node.h"
#include "mparse/ast/cast.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include "util/meta.h"
#include <optional>

namespace ast_ops::matching {

template<typename E>
struct matcher_traits {
  static_assert(util::always_false<E>, "Unknown expression type");
};

template<>
struct matcher_traits<literal_expr> {
  using match_type = mparse::literal_node;
  
  template<typename Ctx>
  static bool match(const literal_expr& expr, const mparse::ast_node_ptr& node, Ctx&) {
    if (auto* lit_node = mparse::ast_node_cast<match_type>(node.get())) {
      return lit_node->val() == expr.val;
    }
    return false;
  }
};

template<typename Node>
struct matcher_traits<node_type_expr<Node>> {
  using match_type = Node;

  template<typename Ctx>
  static bool match(const node_type_expr<Node>&, const mparse::ast_node_ptr& node, Ctx&) {
    return mparse::ast_node_cast<match_type>(node.get()) != nullptr;
  }
};

template<typename Pred, typename Lhs, typename Rhs, bool Commute>
struct matcher_traits<binary_op_pred_expr<Pred, Lhs, Rhs, Commute>> {
  using match_type = mparse::binary_op_node;
  using captures = caplist_cat<get_captures_t<Lhs>, get_captures_t<Rhs>>;

  template<typename Ctx>
  static bool match(const binary_op_pred_expr<Pred, Lhs, Rhs, Commute>& expr, const mparse::ast_node_ptr& node, Ctx& ctx) {
    if (auto* bin_node = mparse::ast_node_cast<match_type>(node.get())) {

      if (!Pred{}(bin_node->type())) {
        return false;
      }

      auto node_lhs = bin_node->ref_lhs();
      auto node_rhs = bin_node->ref_rhs();

      if (matcher_traits<Lhs>::match(expr.lhs, node_lhs, ctx)
        && matcher_traits<Rhs>::match(expr.rhs, node_rhs, ctx)) {
        return true;
      }

      if (Commute && matcher_traits<Lhs>::match(expr.lhs, node_rhs, ctx) 
        && matcher_traits<Rhs>::match(expr.rhs, node_lhs, ctx)) {
        return true;
      }

      return false;
    }
    return false;
  }
};

template<typename Node, typename Inner>
struct matcher_traits<unary_expr<Node, Inner>> {
  using match_type = Node;
  using captures = get_captures_t<Inner>;

  template<typename Ctx>
  static bool match(const unary_expr<Node, Inner>& expr, const mparse::ast_node_ptr& node, Ctx& ctx) {
    if (auto* un_node = mparse::ast_node_cast<match_type>(node.get())) {
      return matcher_traits<Inner>::match(expr.inner, un_node->ref_child(), ctx);
    }
    return false;
  }
};

template<typename Pred, typename Inner>
struct matcher_traits<unary_op_pred_expr<Pred, Inner>> {
  using match_type = mparse::unary_op_node;
  using captures = get_captures_t<Inner>;

  template<typename Ctx>
  static bool match(const unary_op_pred_expr<Pred, Inner>& expr, const mparse::ast_node_ptr& node, Ctx& ctx) {
    if (auto* un_node = mparse::ast_node_cast<match_type>(node.get())) {
      if (!Pred{}(un_node->type())) {
        return false;
      }

      return matcher_traits<Inner>::match(expr.inner, un_node->ref_child(), ctx);
    }
    return false;
  }
};

template<typename Expr>
struct matcher_traits<negation_expr<Expr>> {
  // no captures as this is a negation match
  template<typename Ctx>
  static bool match(const negation_expr<Expr>& expr, const mparse::ast_node_ptr& node, Ctx& ctx) {
    return !matcher_traits<Expr>::match(expr.expr, node, ctx);
  }
};

template<typename First, typename Second>
struct matcher_traits<conjunction_expr<First, Second>> {
  using captures = caplist_cat<get_captures_t<First>, get_captures_t<Second>>;

   template<typename Ctx>
   static bool match(const conjunction_expr<First, Second>& expr, const mparse::ast_node_ptr& node, Ctx& ctx) {
     return matcher_traits<First>::match(expr.first, node, ctx)
       && matcher_traits<Second>::match(expr.second, node, ctx);
   }
};

template<typename First, typename Second>
struct matcher_traits<disjunction_expr<First, Second>> {
  // no captures as this is a disjunction match
  template<typename Ctx>
  static bool match(const disjunction_expr<First, Second>& expr, const mparse::ast_node_ptr& node, Ctx& ctx) {
    return matcher_traits<First>::match(expr.first, node, ctx)
      || matcher_traits<Second>::match(expr.second, node, ctx);
  }
};


namespace impl {

template<typename E, typename = void>
struct get_match_type {
  using type = mparse::ast_node;
};

template<typename E>
struct get_match_type<E, std::void_t<typename matcher_traits<E>::match_type>> {
  using type = typename matcher_traits<E>::match_type;
};

template<typename E>
using get_match_type_t = typename get_match_type<E>::type;

}  // namespace impl


template<typename Tag, typename Expr>
struct matcher_traits<capture_expr_impl<Tag, Expr>> {
  using inner_match_type = impl::get_match_type_t<Expr>;
  using captures = caplist_append<get_captures_t<Expr>, capture<Tag, mparse::node_ptr<inner_match_type>>>;

  template<typename Ctx>
  static bool match(const capture_expr_impl<Tag, Expr>& expr, const mparse::ast_node_ptr& node, Ctx& ctx) {
    if (matcher_traits<Expr>::match(expr.expr, node, ctx)) {
      get_result<Tag>(ctx) = mparse::static_ast_node_ptr_cast<inner_match_type>(node);
      return true;
    }
    return false;
  }
};

template<char C, typename Comp>
struct matcher_traits<subexpr_expr<C, Comp>> {
  using captures = caplist<capture<subexpr_expr_tag<C>, mparse::ast_node_ptr>>;

  template<typename Ctx>
  static bool match(const subexpr_expr<C, Comp>& expr, const mparse::ast_node_ptr& node, Ctx& ctx) {
    auto& saved = get_subexpr<C>(ctx);
    
    if (!saved) {
      saved = node;
      return true;
    }
    
    return compare_exprs(saved.get(), node.get(), expr.comp);
  }
};


template<typename Matcher, typename = std::enable_if_t<is_match_expr<Matcher>>>
std::optional<match_results_for<Matcher>> exec_match(Matcher&& matcher, const mparse::ast_node_ptr& node) {
  match_results_for<Matcher> res;
  if (matcher.matches(node, res)) {
    return res;
  }
  return std::nullopt;
}

}  // namespace ast_opse::matching