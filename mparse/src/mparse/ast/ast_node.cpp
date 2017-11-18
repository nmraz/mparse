#include "ast_node.h"

namespace mparse {

void ast_node::apply_visitor(ast_visitor& vis) {
  vis.visit(*this);
}

}  // namespace mparse