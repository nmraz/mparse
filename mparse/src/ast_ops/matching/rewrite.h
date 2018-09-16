#pragma once

#include "ast_ops/matching/build.h"
#include "ast_ops/matching/match.h"
#include "ast_ops/matching/match_results.h"
#include "mparse/ast/ast_node.h"
#include <functional>

namespace ast_ops::matching {

template<typename M, typename B>
bool rewrite(mparse::ast_node_ptr& node, const M& matcher, const B& builder) {
  if (auto res = exec_match(matcher, node)) {
    node = build_expr(builder, res);
    return true;
  }
  return false;
}


using rewrite_func = std::function<void(mparse::ast_node_ptr&)>;

class rewriter {
public:
  template<typename M, typename B>
  rewriter(const M& matcher, const B& builder);
  rewriter(rewrite_func func);

  void apply(mparse::ast_node_ptr& node) const;

private:
  rewrite_func func_;
};

template<typename M, typename B>
rewriter::rewriter(const M& matcher, const B& builder)
  : rewriter([matcher, builder](mparse::ast_node_ptr& node) {
    rewrite(node, matcher, builder);
  }) {}

}  // namespace ast_ops::matching