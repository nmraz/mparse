#include "paren_node.h"

#include <cassert>
#include <utility>

namespace mparse {

paren_node::paren_node(ast_node_ptr child) {
  set_child(std::move(child));
}

} // namespace mparse