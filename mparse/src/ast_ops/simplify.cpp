#include "simplify.h"

#include "ast_ops/eval/eval.h"
#include "mparse/ast/func_node.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include <algorithm>

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

constexpr matching::rewriter_list canon_rewriters = {
    +x, x,
    x - y, x + -1_lit * y,
    -capture_as<1>(!lit), -1_lit * cap<1>,
    x / y, x * pow(y, -1_lit)
};

constexpr matching::rewriter_list uncanon_rewriters = {
    -1_lit * x, -x,
    x + -y, x - y,

    pow(x, -1_lit), 1_lit / x,
    pow(x, -c1), 1_lit / pow(x, c1),
    x * (1_lit / y), x / y
};

// clang-format on

} // namespace


void canonicalize(mparse::ast_node_ptr& node) {
  strip_parens(node);
  matching::apply_rewriters_recursively(node, canon_rewriters);
}

void uncanonicalize(mparse::ast_node_ptr& node) {
  matching::apply_rewriters_recursively(node, uncanon_rewriters);
}


/* SIMPLIFICATION */

namespace {

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

bool has_constant_args(const mparse::func_node* node) {
  const auto& args = node->args();

  return std::all_of(args.begin(), args.end(), [](const auto& arg) {
    return matching::exec_match(cmplx_lit, arg);
  });
}

bool eval_funcs(mparse::ast_node_ptr& node, const func_scope& fscope) {
  bool changed = false;

  matching::apply_recursively(node, [&](mparse::ast_node_ptr& cur_node) {
    if (auto* func_node =
            mparse::ast_node_cast<const mparse::func_node>(cur_node.get())) {
      if (has_constant_args(func_node)) {
        if (auto* func = fscope.lookup(func_node->name())) {
          cur_node = build_cmplx_lit(eval(cur_node.get(), {}, fscope));
          changed = true;
        }
      }
    }
  });

  return changed;
}


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

constexpr auto one = cmplx_lit_val(1);
constexpr auto zero = cmplx_lit_val(0);

// clang-format off

constexpr matching::rewriter_list const_migrate_rewriters = {
    add_nocomm(capture_as<1>(!cmplx_lit), cc1), cc1 + cap<1>,
    mul_nocomm(capture_as<1>(!cmplx_lit), cc1), cc1 * cap<1>
};

constexpr matching::rewriter_list reassoc_rewriters = {
    add_nocomm(x, y + z), (x + y) + z,
    mul_nocomm(x, y * z), (x * y) * z
};

constexpr matching::rewriter_list simp_rewriters = {
    x + zero, x,

    any * zero, zero,
    x * one, x,

    pow(x, one), x,
    pow(any, zero), one,
    pow(one, any), one,

    x * y + z * x, x * (y + z),
    x + x * y, (one + y) * x,
    x + y * x, (one + y) * x,
    x + x, cmplx_lit_val(2) * x,

    pow(pow(x, y), z), pow(x, y * z),

    pow(x, y) * pow(z, y), pow(x * z, y),

    pow(x, y) * pow(x, z), pow(x, y + z),
    pow(x, y) * x, pow(x, one + y),
    x * x, pow(x, cmplx_lit_val(2))
};

// clang-format on

} // namespace


void simplify(mparse::ast_node_ptr& node, const var_scope& vscope,
              const func_scope& fscope) {
  func_scope eval_fscope = fscope;
  eval_fscope.set_binding(std::string(impl::cmplx_lit_func_name),
                          eval_cmplx_lit);

  canonicalize(node);
  run_with_cmplx_lits(node, [&] {
    propagate_vars(node, vscope);

    bool has_work = true;
    while (has_work) {
      has_work = false;

      while (
          matching::apply_rewriters_recursively(node, const_eval_rewriters)) {
        has_work = true;
      }

      has_work |= eval_funcs(node, eval_fscope);

      while (matching::apply_rewriters_recursively(node,
                                                   const_migrate_rewriters)) {
        has_work = true;
      }

      while (matching::apply_rewriters_recursively(node, reassoc_rewriters)) {
        has_work = true;
      }

      while (matching::apply_rewriters_recursively(node, simp_rewriters)) {
        has_work = true;
      }
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
    cc1,
    build_custom([](auto&& real) { return build_lit(real->val()); },
                 cmplx_lit_real_tag<1>{}) +
        id("i") *
            build_custom([](auto&& imag) { return build_lit(imag->val()); },
                         cmplx_lit_imag_tag<1>{}),
};

// clang-format off

constexpr matching::rewriter_list remove_identity_rewriters = {
    1_lit * x, x,
    0_lit * any, 0_lit,
    x + 0_lit, x
};

// clang-format on

} // namespace

void insert_cmplx_lits(mparse::ast_node_ptr& node) {
  matching::apply_rewriters_recursively(node, insert_cmplx_lit_rewriter);
}

void remove_cmplx_lits(mparse::ast_node_ptr& node) {
  matching::apply_rewriters_recursively(node, remove_cmplx_lit_rewriter);
  matching::apply_rewriters_recursively(node, remove_identity_rewriters);
}

} // namespace impl
} // namespace ast_ops