#include "unary_node.h"

namespace mparse {

void unary_node::set_child(ast_node* child) {
  child_ = child;
  set_parent_for(child_);
}

}  // namespace mparse