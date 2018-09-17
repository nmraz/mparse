#include "strip_parens.h"

#include "ast_ops/matching/rewrite.h"

namespace ast_ops {
namespace {

using namespace ast_ops::matching::literals;

constexpr ast_ops::matching::rewriter_list rewriters = {
  paren(x), x
};

}  // namespace


void strip_parens(mparse::ast_node_ptr& node) {
  ast_ops::matching::apply_rewriters_recursively(node, rewriters);
}

}  // namespace ast_ops