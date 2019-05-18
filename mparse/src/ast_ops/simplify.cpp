#include "simplify.h"

#include "ast_ops/eval/eval.h"
#include "mparse/ast/func_node.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"

using namespace ast_ops::matching::literals;

namespace ast_ops {

/* PARENTHESES */

namespace {

constexpr auto leaf = lit || matching::node_type_expr<mparse::id_node>{};

// clang-format off

constexpr matching::rewriter_list strip_paren_rewriters = {
    paren(x), x
};

constexpr matching::rewriter_list insert_paren_rewriters = {
    capture_as<1>(!(paren(any) || leaf)), paren(cap<1>)
};

// clang-format on

} // namespace


void strip_parens(mparse::ast_node_ptr& node) {
  matching::apply_rewriters_recursively(node, strip_paren_rewriters);
}

void insert_parens(mparse::ast_node_ptr& node) {
  matching::apply_rewriters_recursively(node, insert_paren_rewriters);
}


/* CANONICALIZATION */

namespace {

constexpr auto lit_or_neg = lit || -any;
constexpr auto non_mul = !(any * any || lit_or_neg);


// clang-format off

constexpr matching::rewriter_list canon_op_rewriters = {
    +x, x,
    x - y, x + -1_lit * y,
    -capture_as<1>(!lit), -1_lit * cap<1>,
    x / y, x * pow(y, -1_lit)
};

constexpr matching::rewriter_list canon_mul_ident_rewriter = {
    capture_as<1>(non_mul), 1_lit * cap<1>
};

constexpr matching::rewriter_list canon_pow_ident_rewriter = {
    capture_as<1>(!pow(any, any)), pow(cap<1>, 1_lit)
};

// clang-format on


void canonicalize_mul_ident(mparse::ast_node_ptr& node);

void canonicalize_pow_ident(mparse::ast_node_ptr& node) {
  if (matching::exec_match(non_mul, node)) {
    // insert pow or ignore existing one
    matching::apply_rewriters(node, canon_pow_ident_rewriter);

    matching::apply_to_children(*node, [](mparse::ast_node_ptr& pow_child) {
      matching::apply_to_children(*pow_child, [](mparse::ast_node_ptr& child) {
        canonicalize_mul_ident(child);
      });
    });
  } else {
    // couldn't add a pow here - try children
    matching::apply_to_children(*node, [](mparse::ast_node_ptr& child) {
      canonicalize_pow_ident(child);
    });
  }
}

void canonicalize_mul_ident(mparse::ast_node_ptr& node) {
  matching::apply_rewriters(node, canon_mul_ident_rewriter);

  // expression is of the form `x * ___`
  matching::apply_to_children(*node, [](mparse::ast_node_ptr& mul_child) {
    canonicalize_pow_ident(mul_child);
  });
}


// clang-format off

constexpr matching::rewriter_list uncanon_op_rewriters = {
    -1_lit * x, -x,
    x + -y, x - y,
    
    pow(x, -c1), 1_lit / pow(x, c1),
    x * (1_lit / y), x / y
};

constexpr matching::rewriter_list uncanon_ident_rewriters = {
    0_lit * any, 0_lit,
    x + 0_lit, x,

    pow(any, 0_lit), 1_lit,

    1_lit * x, x,
    pow(x, 1_lit), x
};

// clang-format on


void uncanonicalize_ident(mparse::ast_node_ptr& node) {
  matching::apply_rewriters_recursively(node, uncanon_ident_rewriters);
}

} // namespace


void canonicalize_ops(mparse::ast_node_ptr& node) {
  strip_parens(node);
  matching::apply_rewriters_recursively(node, canon_op_rewriters);
}

void canonicalize(mparse::ast_node_ptr& node) {
  canonicalize_ops(node);
  canonicalize_mul_ident(node);
}


void uncanonicalize_ops(mparse::ast_node_ptr& node) {
  matching::apply_rewriters_recursively(node, uncanon_op_rewriters);
}

void uncanonicalize(mparse::ast_node_ptr& node) {
  uncanonicalize_ident(node); // initial pass
  uncanonicalize_ops(node);
  uncanonicalize_ident(node); // clean up after operator uncanonicalization
}


/* SIMPLIFICATION */

void propagate_vars(mparse::ast_node_ptr& node, const var_scope& vscope) {
  matching::apply_recursively(node, [&](mparse::ast_node_ptr& cur_node) {
    if (auto* id_node =
            mparse::ast_node_cast<mparse::id_node>(cur_node.get())) {
      if (auto val = vscope.lookup(id_node->name())) {
        cur_node = build_cmplx_lit(*val);
      }
    }
  });
}


namespace {

template <typename Lhs, typename Rhs>
constexpr matching::binary_op_pred_expr<matching::always_true_pred, Lhs, Rhs,
                                        false>
match_binop(Lhs lhs, Rhs rhs) {
  return {lhs, rhs};
}

template <typename Inner>
constexpr matching::unary_expr<mparse::unary_node, Inner> match_unop(
    Inner inner) {
  return {inner};
}

number eval_cmplx_lit(util::span<const number> args) {
  return {args[0].real(), args[1].real()};
}

const func_scope lit_eval_func_scope = {
    {std::string(impl::cmplx_lit_func_name), eval_cmplx_lit},
};

constexpr matching::rewriter_list const_eval_rewriters = {
    capture_as<1>(match_unop(cmplx_lit) || match_binop(cmplx_lit, cmplx_lit)),
    build_custom(
        [](auto&& cap) {
          return build_cmplx_lit(eval(cap.get(), {}, lit_eval_func_scope));
        },
        matching::capture_expr_tag<1>{}),
};

} // namespace


void simplify(mparse::ast_node_ptr& node, const var_scope& vscope,
              const func_scope& fscope) {
  canonicalize(node);
  run_with_cmplx_lits(node, [&] {
    propagate_vars(node, vscope);
    while (matching::apply_rewriters_recursively(node, const_eval_rewriters)) {
    }
  });
  uncanonicalize(node);
}


/* MATCHING UTILITIES */

inline namespace simp_matching {

mparse::ast_node_ptr build_cmplx_lit(number val) {
  return mparse::make_ast_node<mparse::func_node>(
      std::string(impl::cmplx_lit_func_name),
      mparse::func_node::arg_list{
          mparse::make_ast_node<mparse::literal_node>(val.real()),
          mparse::make_ast_node<mparse::literal_node>(val.imag())});
}

} // namespace simp_matching

namespace impl {
namespace {

mparse::ast_node_ptr build_lit(double val) {
  if (val < 0) {
    return mparse::make_ast_node<mparse::unary_op_node>(
        mparse::unary_op_type::neg,
        mparse::make_ast_node<mparse::literal_node>(-val));
  }

  return mparse::make_ast_node<mparse::literal_node>(val);
}

constexpr matching::rewriter_list insert_cmplx_lit_rewriter = {
    c1,
    build_custom([](auto&& c1) { return build_cmplx_lit(c1->val()); },
                 matching::constant_expr_tag<1>{}),
};

constexpr matching::rewriter_list remove_cmplx_lit_rewriter = {
    cmplx_lit_cap<1>,
    build_custom([](auto&& real) { return build_lit(real->val()); },
                 cmplx_lit_real_tag<1>{}) +
        id("i") *
            build_custom([](auto&& imag) { return build_lit(imag->val()); },
                         cmplx_lit_imag_tag<1>{}),
};

} // namespace

void insert_cmplx_lits(mparse::ast_node_ptr& node) {
  matching::apply_rewriters_recursively(node, insert_cmplx_lit_rewriter);
}

void remove_cmplx_lits(mparse::ast_node_ptr& node) {
  matching::apply_rewriters_recursively(node, remove_cmplx_lit_rewriter);
  uncanonicalize_ident(node);
}

} // namespace impl
} // namespace ast_ops