#include "abstract_syntax_tree.h"

namespace mparse {

void abstract_syntax_tree::set_root(ast_node* root) {
  root_ = root;
}

}  // namespace mparse