#pragma once

#include "ast_ops/eval/scope.h"
#include "ast_ops/eval/types.h"
#include "ast_ops/matching/match_results.h"
#include "ast_ops/matching/rewrite.h"
#include "mparse/ast/ast_node.h"
#include "mparse/ast/cast.h"
#include "mparse/ast/literal_node.h"
#include <string_view>

namespace ast_ops {

void strip_parens(mparse::ast_node_ptr& node);
void insert_parens(mparse::ast_node_ptr& node);


void canonicalize_ops(mparse::ast_node_ptr& node);
void canonicalize(mparse::ast_node_ptr& node);

void uncanonicalize_ops(mparse::ast_node_ptr& node);
void uncanonicalize(mparse::ast_node_ptr& node);



void simplify(mparse::ast_node_ptr& node, const var_scope& vscope = {},
              const func_scope& fscope = {});


inline namespace simp_matching {
template <int N>
struct cmplx_lit_tag {};

namespace impl {

constexpr std::string_view cmplx_lit_func_name = "$l";

template <int N, typename Expr>
constexpr matching::capture_expr_impl<cmplx_lit_tag<N>, Expr>
capture_as_cmplx_lit(Expr expr) {
  return {expr};
}

} // namespace impl


constexpr auto cmplx_lit_val(number val) {
  return func(impl::cmplx_lit_func_name, matching::literal_expr{val.real()},
              matching::literal_expr{val.imag()});
}

constexpr auto cmplx_lit =
    func(impl::cmplx_lit_func_name, matching::lit, matching::lit);

template <int N>
constexpr auto cmplx_lit_cap = impl::capture_as_cmplx_lit<N>(cmplx_lit);

template <int N, typename Res>
number get_cmplx_lit_val(Res&& results) {
  const auto& args =
      matching::get_result<cmplx_lit_tag<N>>(std::forward<Res>(results))
          ->args();

  return {static_cast<const mparse::literal_node*>(args[0].get())->val(),
          static_cast<const mparse::literal_node*>(args[1].get())->val()};
}


mparse::ast_node_ptr build_cmplx_lit(number val);

} // namespace simp_matching
} // namespace ast_ops