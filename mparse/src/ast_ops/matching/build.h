#pragma once

#include "ast_ops/matching/expr.h"
#include "ast_ops/matching/match_results.h"
#include "mparse/ast/ast_node.h"
#include "mparse/ast/func_node.h"
#include "mparse/ast/id_node.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include "util/meta.h"

namespace ast_ops::matching {

template<typename E>
struct builder_traits {
  static_assert(util::always_false<E>, "Unknown expression type");
};

template<typename F>
struct builder_traits<custom_builder_expr<F>> {
  template<typename BuildTags, typename Ctx>
  static auto build(const custom_builder_expr<F>& expr, const Ctx& ctx) {
    return expr.func(ctx);
  }
};

template<>
struct builder_traits<literal_expr> {
  template<typename BuildTags, typename Ctx>
  static auto build(const literal_expr& expr, const Ctx&) {
    return mparse::make_ast_node<mparse::literal_node>(expr.val);
  }
};

template<>
struct builder_traits<id_expr> {
  template<typename BuildTags, typename Ctx>
  static auto build(const id_expr& expr, const Ctx&) {
    return mparse::make_ast_node<mparse::id_node>(std::string(expr.name));
  }
};

template<typename... Args>
struct builder_traits<func_expr<Args...>> {
private:
  template<typename BuildTags, typename... Args, size_t... I, typename Ctx>
  static mparse::func_node::arg_list build_args(const std::tuple<Args...>& args,
    std::index_sequence<I...>, Ctx& ctx) {
    mparse::func_node::arg_list arg_nodes;
    (..., arg_nodes.push_back(builder_traits<Args>::template build<BuildTags>(std::get<I>(args), ctx)));
    return arg_nodes;
  }

public:
  template<typename BuildTags, typename Ctx>
  static auto build(const func_expr<Args...>& expr, const Ctx& ctx) {
    return mparse::make_ast_node<mparse::func_node>(
      std::string(expr.name),
      build_args<BuildTags>(expr.args, std::index_sequence_for<Args...>{}, ctx)
    );
  }
};

template<typename Node, typename Inner>
struct builder_traits<unary_expr<Node, Inner>> {
  template<typename BuildTags, typename Ctx>
  static auto build(const unary_expr<Node, Inner>& expr, const Ctx& ctx) {
    auto inner_node = builder_traits<Inner>::template build<BuildTags>(expr.inner, ctx);
    return mparse::make_ast_node<Node>(std::move(inner_node));
  }
};

template<mparse::unary_op_type Type, typename Inner>
struct builder_traits<unary_op_expr<Type, Inner>> {
  template<typename BuildTags, typename Ctx>
  static auto build(const unary_op_expr<Type, Inner>& expr, const Ctx& ctx) {
    auto inner_node = builder_traits<Inner>::template build<BuildTags>(expr.inner, ctx);
    return mparse::make_ast_node<mparse::unary_op_node>(Type, std::move(inner_node));
  }
};

template<mparse::binary_op_type Type, typename Lhs, typename Rhs, bool Commute>
struct builder_traits<binary_op_expr<Type, Lhs, Rhs, Commute>> {
  template<typename BuildTags, typename Ctx>
  static auto build(const binary_op_expr<Type, Lhs, Rhs, Commute>& expr, const Ctx& ctx) {
    auto lhs_node = builder_traits<Lhs>::template build<BuildTags>(expr.lhs, ctx);
    auto rhs_node = builder_traits<Rhs>::template build<BuildTags>(expr.rhs, ctx);

    return mparse::make_ast_node<mparse::binary_op_node>(Type, std::move(lhs_node),
      std::move(rhs_node));
  }
};

template<typename Tag, typename Expr>
struct builder_traits<capture_expr_impl<Tag, Expr>> {
  template<typename BuildTags, typename Ctx>
  static auto build(const capture_expr_impl<Tag, Expr>& expr, const Ctx& ctx) {
    return get_result<Tag>(ctx);
  }
};

template<char C, typename Comp>
struct builder_traits<subexpr_expr<C, Comp>> {
  template<typename BuildTags, typename Ctx>
  static auto build(const subexpr_expr<C, Comp>&, const Ctx& ctx) {
    return get_subexpr<C>(ctx);
  }
};


namespace impl {

template<typename E, typename = void>
struct get_build_tags {
  using type = caplist<>;
};

template<typename E>
struct get_build_tags<E, std::void_t<typename builder_traits<E>::tags>> {
  using type = typename builder_traits<E>::tags;
};

template<typename E>
using get_build_tags_t = typename get_build_tags<E>::type;

}  // namespace impl


template<typename Expr, typename Ctx>
mparse::ast_node_ptr build_expr(Expr expr, const Ctx& ctx) {
  using build_tags = impl::get_build_tags_t<Expr>;
  return builder_traits<Expr>::template build<build_tags>(expr, ctx);
}

}  // namespace ast_ops::matching