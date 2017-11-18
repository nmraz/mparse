#include "abstract_syntax_tree.h"

#include <iterator>
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

void abstract_syntax_tree::take_nodes(abstract_syntax_tree&& rhs) {
  nodes_.insert(
    nodes_.end(),
    std::make_move_iterator(rhs.nodes_.begin()),
    std::make_move_iterator(rhs.nodes_.end())
  );
  rhs.root_ = nullptr;
}

void abstract_syntax_tree::set_root(ast_node* root) {
  root_ = root;
}

}  // namespace mparse