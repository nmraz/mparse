#include "unary_node.h"

namespace mparse {

void unary_node::set_child(ast_node* child) {
  child_ = child;
  child_->set_parent(this);
}

}  // namespace mparse