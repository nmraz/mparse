#pragma once

#include "ast_ops/matching/build.h"
#include "ast_ops/matching/match.h"
#include "ast_ops/matching/match_results.h"
#include "mparse/ast/ast_node.h"
#include <functional>
#include <tuple>
#include <type_traits>

namespace ast_ops::matching {

template<typename M, typename B>
bool rewrite(mparse::ast_node_ptr& node, const M& matcher, const B& builder) {
  if (auto res = exec_match(matcher, node)) {
    node = build_expr(builder, std::move(*res));
    return true;
  }
  return false;
}


namespace impl {

template<typename M, typename B, typename... Rest>
constexpr auto get_rewriters_after_expr(const M& matcher, const B& builder, const Rest&... rest);

constexpr auto get_rewriters() {  // base case
  return std::tuple{};
}

template<typename First, typename... Rest>
constexpr auto get_rewriters(const First& first, const Rest&... rest) {
  if constexpr (is_match_expr<First>) {
    return get_rewriters_after_expr(first, rest...);
  } else {
    static_assert(std::is_invocable_r_v<bool, First, mparse::ast_node_ptr&>, "Expected a rewriter function");
    return std::tuple_cat(std::tuple{ first }, get_rewriters(rest...));
  }
}


template<typename M, typename B>
constexpr auto get_expr_rewriter(const M& matcher, const B& builder) {
  return [matcher, builder] (mparse::ast_node_ptr& node) {
    return rewrite(node, matcher, builder);
  };
}

template<typename M, typename B, typename... Rest>
constexpr auto get_rewriters_after_expr(const M& matcher, const B& builder, const Rest&... rest) {
  static_assert(is_match_expr<B>, "Expected a builder after match expression");
  return std::tuple_cat(std::tuple{ get_expr_rewriter(matcher, builder) }, get_rewriters(rest...));
}


template<typename... Ts>
bool do_apply_rewriters(mparse::ast_node_ptr& node, const Ts&... rewriters) {
  // note: not || as we don't want short-circuiting
  return (... | rewriters(node));
}

}  // namespace impl


template<typename... Ts>
class rewriter_list {
public:
  constexpr rewriter_list(const Ts&... args) : rewriters_(impl::get_rewriters(args...)) {}

  template<typename... Us>
  friend bool apply_rewriters(mparse::ast_node_ptr& node, const rewriter_list<Us...>& list);

private:
  decltype(impl::get_rewriters(std::declval<const Ts&>()...)) rewriters_;
};

template<typename... Ts>
bool apply_rewriters(mparse::ast_node_ptr& node, const rewriter_list<Ts...>& list) {
  return std::apply([&] (auto&&... rewriters) {
    return impl::do_apply_rewriters(node, rewriters...);
  }, list.rewriters_);
}


using rewriter_func = std::function<bool(mparse::ast_node_ptr&)>;

bool apply_recursively(mparse::ast_node_ptr& node, rewriter_func func);

template<typename... Ts>
bool apply_rewriters_recursively(mparse::ast_node_ptr& node, const rewriter_list<Ts...>& list) {
  return apply_recursively(node, [&] (mparse::ast_node_ptr& cur_node) {
    return apply_rewriters(cur_node, list);
  });
}

}  // namespace ast_ops::matching