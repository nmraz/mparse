#include "unary_node.h"

namespace mparse {

void unary_node::set_child(ast_node* child) {
  child_ = child;
}

}  // namespace mparse