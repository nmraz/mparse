#pragma once

#include "ast_ops/clone.h"
#include "ast_ops/matching/expr.h"
#include "ast_ops/matching/match_results.h"
#include "mparse/ast/ast_node.h"
#include "mparse/ast/func_node.h"
#include "mparse/ast/id_node.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include "util/meta.h"
#include <utility>

namespace ast_ops::matching {

template<typename E>
struct builder_traits {
  static_assert(util::always_false<E>, "Unknown expression type");
};


namespace impl {

template<typename E, typename = void>
struct get_build_tags {
  using type = util::type_list<>;
};

template<typename E>
struct get_build_tags<E, std::void_t<typename builder_traits<E>::tags>> {
  using type = typename builder_traits<E>::tags;
};

template<typename E>
using get_build_tags_t = typename get_build_tags<E>::type;

}  // namespace impl


template<typename F>
struct builder_traits<custom_builder_expr<F>> {
  template<typename BuildTags, typename Ctx>
  static auto build(const custom_builder_expr<F>& expr, Ctx&& ctx) {
    return expr.func(std::forward<Ctx>(ctx));
  }
};

template<>
struct builder_traits<literal_expr> {
  template<typename BuildTags, typename Ctx>
  static auto build(const literal_expr& expr, Ctx&&) {
    return mparse::make_ast_node<mparse::literal_node>(expr.val);
  }
};

template<>
struct builder_traits<id_expr> {
  template<typename BuildTags, typename Ctx>
  static auto build(const id_expr& expr, Ctx&&) {
    return mparse::make_ast_node<mparse::id_node>(std::string(expr.name));
  }
};

template<typename... Args>
struct builder_traits<func_expr<Args...>> {
private:
  template<typename BuildTags, typename... Args, size_t... I, typename Ctx>
  static mparse::func_node::arg_list build_args(const std::tuple<Args...>& args,
    std::index_sequence<I...>, Ctx&& ctx) {
    mparse::func_node::arg_list arg_nodes;
    (..., arg_nodes.push_back(builder_traits<Args>::template build<BuildTags>(std::get<I>(args), std::forward<Ctx>(ctx))));
    return arg_nodes;
  }

public:
  using tags = util::type_list_cat_t<impl::get_build_tags_t<Args>...>;

  template<typename BuildTags, typename Ctx>
  static auto build(const func_expr<Args...>& expr, Ctx&& ctx) {
    return mparse::make_ast_node<mparse::func_node>(
      std::string(expr.name),
      build_args<BuildTags>(expr.args, std::index_sequence_for<Args...>{}, std::forward<Ctx>(ctx))
    );
  }
};

template<typename Node, typename Inner>
struct builder_traits<unary_expr<Node, Inner>> {
  using tags = impl::get_build_tags_t<Inner>;

  template<typename BuildTags, typename Ctx>
  static auto build(const unary_expr<Node, Inner>& expr, Ctx&& ctx) {
    auto inner_node = builder_traits<Inner>::template build<BuildTags>(expr.inner, std::forward<Ctx>(ctx));
    return mparse::make_ast_node<Node>(std::move(inner_node));
  }
};

template<mparse::unary_op_type Type, typename Inner>
struct builder_traits<unary_op_expr<Type, Inner>> {
  using tags = impl::get_build_tags_t<Inner>;

  template<typename BuildTags, typename Ctx>
  static auto build(const unary_op_expr<Type, Inner>& expr, Ctx&& ctx) {
    auto inner_node = builder_traits<Inner>::template build<BuildTags>(expr.inner, std::forward<Ctx>(ctx));
    return mparse::make_ast_node<mparse::unary_op_node>(Type, std::move(inner_node));
  }
};

template<mparse::binary_op_type Type, typename Lhs, typename Rhs, bool Commute>
struct builder_traits<binary_op_expr<Type, Lhs, Rhs, Commute>> {
  using tags = util::type_list_cat_t<impl::get_build_tags_t<Lhs>, impl::get_build_tags_t<Rhs>>;

  template<typename BuildTags, typename Ctx>
  static auto build(const binary_op_expr<Type, Lhs, Rhs, Commute>& expr, Ctx&& ctx) {
    auto lhs_node = builder_traits<Lhs>::template build<BuildTags>(expr.lhs, std::forward<Ctx>(ctx));
    auto rhs_node = builder_traits<Rhs>::template build<BuildTags>(expr.rhs, std::forward<Ctx>(ctx));

    return mparse::make_ast_node<mparse::binary_op_node>(Type, std::move(lhs_node),
      std::move(rhs_node));
  }
};


namespace impl {

template<typename BuildTags, typename Tag, typename Ctx>
auto get_build_result(Ctx&& ctx) {
  auto&& stored = get_result<Tag>(std::forward<Ctx>(ctx));

  if constexpr (util::type_list_count_v<Tag, BuildTags> > 1) {
    // used several times - clone for safety
    return ast_ops::clone(stored.get());
  } else {
    // only required once - no need to copy
    return std::forward<decltype(stored)>(stored);
  }
}

}  // namespace impl

template<typename Tag, typename Expr>
struct builder_traits<capture_expr_impl<Tag, Expr>> {
  using tags = util::type_list_append_t<impl::get_build_tags_t<Expr>, Tag>;

  template<typename BuildTags, typename Ctx>
  static auto build(const capture_expr_impl<Tag, Expr>& expr, Ctx&& ctx) {
    return impl::get_build_result<BuildTags, Tag>(std::forward<Ctx>(ctx));
  }
};

template<char C, typename Comp>
struct builder_traits<subexpr_expr<C, Comp>> {
  using tags = util::type_list<subexpr_expr_tag<C>>;

  template<typename BuildTags, typename Ctx>
  static auto build(const subexpr_expr<C, Comp>&, Ctx&& ctx) {
    return impl::get_build_result<BuildTags, subexpr_expr_tag<C>>(std::forward<Ctx>(ctx));
  }
};


template<typename Expr, typename Ctx>
mparse::ast_node_ptr build_expr(Expr expr, Ctx& ctx) {
  using build_tags = impl::get_build_tags_t<Expr>;
  return builder_traits<Expr>::template build<build_tags>(expr, std::forward<Ctx>(ctx));
}

}  // namespace ast_ops::matching