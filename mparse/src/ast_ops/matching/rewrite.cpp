#include "rewrite.h"

#include <utility>

namespace ast_ops::matching {

rewriter::rewriter(rewrite_func func)
  : func_(std::move(func)) {
}

void rewriter::apply(mparse::ast_node_ptr& node) const {
  func_(node);
}

}  // namespace ast_ops::matching