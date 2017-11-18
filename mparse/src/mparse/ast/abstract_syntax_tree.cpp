#include "abstract_syntax_tree.h"

#include <utility>

namespace mparse {

abstract_syntax_tree::abstract_syntax_tree(abstract_syntax_tree&& rhs)
  : nodes_(std::move(rhs.nodes_))
  , root_(std::exchange(rhs.root_, nullptr)) {
}

abstract_syntax_tree& abstract_syntax_tree::operator=(abstract_syntax_tree&& rhs) {
  nodes_ = std::move(rhs.nodes_);
  root_ = std::exchange(rhs.root_, nullptr);
  return *this;
}

void abstract_syntax_tree::set_root(ast_node* root) {
  root_ = root;
}

}  // namespace mparse