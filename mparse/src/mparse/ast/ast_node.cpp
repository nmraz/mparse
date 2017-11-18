#include "ast_node.h"

namespace mparse {

void ast_node::apply_visitor(ast_visitor& vis) {
  vis.visit(*this);
}

void ast_node::apply_visitor(const_ast_visitor& vis) const {
  vis.visit(*this);
}

}  // namespace mparse