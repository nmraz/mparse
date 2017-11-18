#include "paren_node.h"
#include <cassert>

namespace mparse {

paren_node::paren_node(ast_node* child) {
  set_child(child);
}

}  // namespace mparse