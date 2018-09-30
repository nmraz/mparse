#include "simplify.h"

#include "ast_ops/matching/rewrite.h"

using namespace ast_ops::matching::literals;

namespace ast_ops {
namespace {

// clang-format off

constexpr ast_ops::matching::rewriter_list strip_paren_rewriters = {
    paren(x), x
};


constexpr ast_ops::matching::rewriter_list canon_op_rewriters = {
    +x, x,

    x - y, x + -y,
    
    -capture_as<1>(match_not(lit)), -1_lit * cap<1>,
    
    x / y, x * pow(y, -1_lit)
};


constexpr auto lit_or_neg = match_or(lit, -any);

constexpr ast_ops::matching::rewriter_list canon_rewriters = {
    capture_as<1>(match_not(match_or(match_or(pow(x, y), x * y), lit_or_neg))),
    pow(cap<1>, 1_lit), // x -> x ^ 1

    capture_as<1>(match_not(match_or(x * y, lit_or_neg))),
    1_lit * cap<1> // x -> 1 * x
};

// clang-format on


void do_canonicalize(mparse::ast_node_ptr& node) {
  matching::apply_rewriters(node, canon_rewriters);

  // At this point, the expression is of the form `lit * (___ ^ lit)`. We now
  // need to canonicalize the children of `___`.
  matching::apply_to_children(*node, [](mparse::ast_node_ptr& mul_child) {
    matching::apply_to_children(
        *mul_child, [](mparse::ast_node_ptr& pow_child) {
          matching::apply_to_children(
              *pow_child,
              [](mparse::ast_node_ptr& child) { do_canonicalize(child); });
        });
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

void decanonicalize(mparse::ast_node_ptr& node) {}

} // namespace ast_ops