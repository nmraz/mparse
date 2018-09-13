#pragma once

#include "ast_ops/matching/build.h"
#include "ast_ops/matching/match.h"
#include "ast_ops/matching/match_results.h"
#include "mparse/ast/ast_node.h"

namespace ast_ops::matching {

template<typename M, typename B>
void rewrite(mparse::ast_node_ptr& node, const M& matcher, const B& builder) {
  if (auto res = exec_match(matcher, node)) {
    node = build_expr(builder, res);
  }
}

}  // namespace ast_ops::matching