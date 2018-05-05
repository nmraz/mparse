#pragma once

#include "ast_ops/matching/match_expr.h"
#include "ast_ops/matching/match_results.h"
#include "mparse/ast/ast_node.h"
#include <optional>

namespace ast_ops::matching {

template<typename Matcher, typename = std::enable_if_t<is_match_expr<Matcher>>>
std::optional<match_results_for<Matcher>> exec_match(Matcher&& matcher, const mparse::ast_node_ptr& node) {
  match_results_for<Matcher> res;
  if (matcher.matches(node, res)) {
    return res;
  }
  return std::nullopt;
}

}  // namespace ast_opse::matching