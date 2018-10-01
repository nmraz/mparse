#include "simplify.h"

#include "ast_ops/matching/rewrite.h"

using namespace ast_ops::matching::literals;

namespace ast_ops {
namespace {

/* CANONICALIZATION */

constexpr auto lit_or_neg = match_or(lit, -any);
constexpr auto non_mul = match_not(match_or(any * any, lit_or_neg));


// clang-format off

constexpr matching::rewriter_list strip_paren_rewriters = {
    paren(x), x
};


constexpr matching::rewriter_list canon_op_rewriters = {
    +x, x,
    x - y, x + -y,
    -capture_as<1>(match_not(lit)), -1_lit * cap<1>,
    x / y, x * pow(y, -1_lit)
};

constexpr matching::rewriter_list canon_mul_rewriter = {
    capture_as<1>(non_mul), 1_lit * cap<1>
};

constexpr matching::rewriter_list canon_pow_rewriter = {
    capture_as<1>(match_not(pow(any, any))), pow(cap<1>, 1_lit)
};

// clang-format on


void do_canonicalize(mparse::ast_node_ptr& node);

void do_canonicalize_pow(mparse::ast_node_ptr& node) {
  if (matching::exec_match(non_mul, node)) {
    // insert pow or ignore existing one
    matching::apply_rewriters(node, canon_pow_rewriter);

    matching::apply_to_children(*node, [](mparse::ast_node_ptr& pow_child) {
      matching::apply_to_children(*pow_child, [](mparse::ast_node_ptr& child) {
        do_canonicalize(child);
      });
    });
  } else {
    // couldn't add a pow here - try children
    matching::apply_to_children(
        *node, [](mparse::ast_node_ptr& child) { do_canonicalize_pow(child); });
  }
}

void do_canonicalize(mparse::ast_node_ptr& node) {
  matching::apply_rewriters(node, canon_mul_rewriter);

  // expression is of the form `x * ___`
  matching::apply_to_children(*node, [](mparse::ast_node_ptr& mul_child) {
    do_canonicalize_pow(mul_child);
  });
}

} // namespace


void strip_parens(mparse::ast_node_ptr& node) {
  matching::apply_rewriters_recursively(node, strip_paren_rewriters);
}


void canonicalize_ops(mparse::ast_node_ptr& node) {
  strip_parens(node);
  while (matching::apply_rewriters_recursively(node, canon_op_rewriters)) {
  }
}

void canonicalize(mparse::ast_node_ptr& node) {
  canonicalize_ops(node);
  do_canonicalize(node);
}


void uncanonicalize_ops(mparse::ast_node_ptr&) {}

void uncanonicalize(mparse::ast_node_ptr&) {}

} // namespace ast_ops