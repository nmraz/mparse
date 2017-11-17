#include "ast_node.h"

namespace mparse {

void ast_node::apply_visitor(ast_visitor& vis) {
  vis.visit(*this);
}


void ast_node::set_parent_for(ast_node* child) {
  child->parent_ = this;
}

}  // namespace mparse