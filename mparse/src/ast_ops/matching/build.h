#pragma once

#include "ast_ops/matching/expr.h"
#include "mparse/ast/ast_node.h"
#include "util/meta.h"

namespace ast_ops::matching {

template<typename E>
struct builder_traits {
  static_assert(util::always_false<E>, "Unknown expression type");
};


template<typename Expr, typename Ctx>
mparse::ast_node_ptr build_expr(Expr expr, const Ctx& ctx) {
  return builder_traits<Expr>::build(ctx);
}

}  // namespace ast_ops::matching

