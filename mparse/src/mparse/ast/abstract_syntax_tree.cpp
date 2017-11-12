#include "abstract_syntax_tree.h"

namespace mparse {

abstract_syntax_tree::abstract_syntax_tree() {
  make_node<root_node>();
}

root_node* abstract_syntax_tree::root() {
  // the root node is always the first one
  return static_cast<root_node*>(nodes_.front().get());
}

const root_node* abstract_syntax_tree::root() const {
  return static_cast<const root_node*>(nodes_.front().get());
}

}  // namespace mparse