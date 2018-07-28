#pragma once

#include "ast_ops/matching/match_expr.h"
#include "ast_ops/matching/match_results.h"
#include "mparse/ast/ast_node.h"
#include <optional>

namespace ast_ops::matching {
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


template<typename Matcher, typename = std::enable_if_t<is_match_expr<Matcher>>>
std::optional<match_results_for<Matcher>> exec_match(Matcher&& matcher, const mparse::ast_node_ptr& node) {
  match_results_for<Matcher> res;
  if (matcher.matches(node, res)) {
    return res;
  }
  return std::nullopt;
}

}  // namespace ast_opse::matching