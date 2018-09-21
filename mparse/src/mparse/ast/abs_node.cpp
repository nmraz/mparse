#include "abs_node.h"

namespace mparse {

abs_node::abs_node(ast_node_ptr child) {
  set_child(std::move(child));
}

} // namespace mparse