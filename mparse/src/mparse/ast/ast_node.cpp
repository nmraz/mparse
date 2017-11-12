#include "ast_node.h"

namespace mparse {

void ast_node::set_parent(ast_node* parent) {
  parent_ = parent;
}

void ast_node::apply_visitor(ast_visitor& vis) {
  vis.visit(*this);
}

}  // namespace mparse