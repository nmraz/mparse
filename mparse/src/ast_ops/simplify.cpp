#include "simplify.h"

#include "ast_ops/matching/rewrite.h"

using namespace ast_ops::matching::literals;

namespace ast_ops {
namespace {

// clang-format off

constexpr ast_ops::matching::rewriter_list strip_paren_rewriters = {
    paren(x),
    x, // (x) -> x
};


constexpr ast_ops::matching::rewriter_list canon_op_rewriters = {
    +x,
    x, // +x -> x

    x - y,
    x + -y, // x - y -> x + -y

    -capture_as<1>(match_not(lit)),
    -1_lit * cap<1>, // -x -> -1 * x

    x / y,
    x * pow(y, -1_lit) // x / y -> x * y^-1
};


constexpr auto lit_or_neg = match_or(lit, -any);

constexpr ast_ops::matching::rewriter_list canon_rewriters = {
    capture_as<1>(match_not(match_or(match_or(pow(x, y), x * y), lit_or_neg))),
    pow(cap<1>, 1_lit), // x -> x ^ 1

    capture_as<1>(match_not(match_or(x * y, lit_or_neg))),
    1_lit * cap<1> // x -> 1 * x
};

// clang-format on

} // namespace


void strip_parens(mparse::ast_node_ptr& node) {
  matching::apply_rewriters_recursively(node, strip_paren_rewriters);
}


void canonicalize(mparse::ast_node_ptr& node) {
  matching::apply_rewriters_recursively(node, canon_op_rewriters);
  matching::apply_rewriters_recursively(node, canon_rewriters);
}

void decanonicalize(mparse::ast_node_ptr& node) {}

} // namespace ast_ops