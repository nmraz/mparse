#pragma once

#include "ast_ops/eval/scope.h"
#include "ast_ops/eval/types.h"
#include "ast_ops/matching/match_results.h"
#include "ast_ops/matching/rewrite.h"
#include "mparse/ast/ast_node.h"
#include "mparse/ast/cast.h"
#include "mparse/ast/literal_node.h"
#include "util/finally.h"
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
struct cmplx_lit_real_tag {};

template <int N>
struct cmplx_lit_imag_tag {};

} // namespace simp_matching

namespace impl {

constexpr std::string_view cmplx_lit_func_name = "$l";

template <typename R, typename I>
constexpr auto cmplx_lit_expr(R real, I imag) {
  return func(cmplx_lit_func_name, real, imag);
}

} // namespace impl

inline namespace simp_matching {

constexpr auto cmplx_lit_val(number val) {
  return impl::cmplx_lit_expr(matching::literal_expr{val.real()},
                              matching::literal_expr{val.imag()});
}

constexpr auto cmplx_lit = impl::cmplx_lit_expr(matching::lit, matching::lit);

template <int N>
constexpr auto cmplx_lit_cap = impl::cmplx_lit_expr(
    matching::capture_as_impl<cmplx_lit_real_tag<N>>(matching::lit),
    matching::capture_as_impl<cmplx_lit_imag_tag<N>>(matching::lit));

constexpr auto cc1 = cmplx_lit_cap<1>;
constexpr auto cc2 = cmplx_lit_cap<2>;
constexpr auto cc3 = cmplx_lit_cap<3>;

mparse::ast_node_ptr build_cmplx_lit(number val);

} // namespace simp_matching

namespace impl {

void insert_cmplx_lits(mparse::ast_node_ptr& node);
void remove_cmplx_lits(mparse::ast_node_ptr& node);

} // namespace impl

template <typename F>
void run_with_cmplx_lits(mparse::ast_node_ptr& node, F&& func) {
  impl::insert_cmplx_lits(node);
  util::finally remove_cmplx([&] { impl::remove_cmplx_lits(node); });

  std::forward<F>(func)();
}

} // namespace ast_ops