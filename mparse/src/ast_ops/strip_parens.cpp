#include "strip_parens.h"

#include "ast_ops/matching/rewrite.h"

namespace ast_ops {

void strip_parens(mparse::ast_node_ptr& node) {
  using namespace ast_ops::matching::literals;

  static constexpr ast_ops::matching::rewriter_list rewriters = {
      paren(x),
      x,
  };

  ast_ops::matching::apply_rewriters_recursively(node, rewriters);
}

} // namespace ast_ops