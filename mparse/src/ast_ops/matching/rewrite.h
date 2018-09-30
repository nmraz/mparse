#pragma once

#include "ast_ops/matching/build.h"
#include "ast_ops/matching/match.h"
#include "ast_ops/matching/match_results.h"
#include "mparse/ast/ast_node.h"
#include <functional>
#include <tuple>
#include <type_traits>

namespace ast_ops::matching {

template <typename M, typename B>
bool rewrite(mparse::ast_node_ptr& node, const M& matcher, const B& builder) {
  if (auto res = exec_match(matcher, node)) {
    node = build_expr(builder, std::move(*res));
    return true;
  }
  return false;
}


namespace impl {

template <typename M, typename B, typename... Rest>
constexpr auto get_rewriters_after_expr(const M& matcher, const B& builder,
                                        const Rest&... rest);

constexpr auto get_rewriters() { // base case
  return std::tuple{};
}

template <typename First, typename... Rest>
constexpr auto get_rewriters(const First& first, const Rest&... rest) {
  if constexpr (is_match_expr<First>) {
    return get_rewriters_after_expr(first, rest...);
  } else {
    static_assert(std::is_invocable_r_v<bool, First, mparse::ast_node_ptr&>,
                  "Expected a rewriter function");
    return std::tuple_cat(std::tuple{first}, get_rewriters(rest...));
  }
}


template <typename M, typename B>
constexpr auto get_expr_rewriter(const M& matcher, const B& builder) {
  return [matcher, builder](mparse::ast_node_ptr& node) {
    return rewrite(node, matcher, builder);
  };
}

template <typename M, typename B, typename... Rest>
constexpr auto get_rewriters_after_expr(const M& matcher, const B& builder,
                                        const Rest&... rest) {
  static_assert(is_match_expr<B>, "Expected a builder after match expression");
  return std::tuple_cat(std::tuple{get_expr_rewriter(matcher, builder)},
                        get_rewriters(rest...));
}

} // namespace impl


template <typename... Ts>
class rewriter_list {
public:
  constexpr rewriter_list(const Ts&... args)
      : rewriters_(impl::get_rewriters(args...)) {}

  template <typename... Us>
  friend bool apply_rewriters(mparse::ast_node_ptr& node,
                              const rewriter_list<Us...>& list);

private:
  decltype(impl::get_rewriters(std::declval<const Ts&>()...)) rewriters_;
};

template <typename... Ts>
bool apply_rewriters(mparse::ast_node_ptr& node,
                     const rewriter_list<Ts...>& list) {
  return std::apply(
      [&](auto&&... rewriters) {
        bool ret = false;

        // no short-circuiting, executed in order
        ((ret |= rewriters(node)), ...);
        return ret;
      },
      list.rewriters_);
}


using rewriter_func = std::function<void(mparse::ast_node_ptr&)>;

void apply_to_children(mparse::ast_node& node, const rewriter_func& func);

template <typename F>
void apply_recursively(mparse::ast_node_ptr& node, F&& func) {
  apply_to_children(*node, [&](mparse::ast_node_ptr& cur_node) {
    apply_recursively(cur_node, std::forward<F>(func));
  });
  std::forward<F>(func)(node);
}

template <typename... Ts>
bool apply_rewriters_recursively(mparse::ast_node_ptr& node,
                                 const rewriter_list<Ts...>& list) {
  bool ret = false;
  apply_recursively(node, [&](mparse::ast_node_ptr& cur_node) {
    ret |= apply_rewriters(cur_node, list);
  });
  return ret;
}

} // namespace ast_ops::matching