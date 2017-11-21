#include "unary_node.h"

#include <utility>

namespace mparse {

void unary_node::set_child(ast_node_ptr child) {
  child_ = std::move(child);
}

ast_node_ptr unary_node::take_child() {
  return std::move(child_);
}

}  // namespace mparse