#include "ast_node.h"

namespace mparse {

ast_node::~ast_node() { // definition provided because base dtors are always
                        // called (even when pure virtual)
}

void ast_node::apply_visitor(ast_visitor& vis) {
  vis.visit(*this);
}

void ast_node::apply_visitor(const_ast_visitor& vis) const {
  vis.visit(*this);
}

} // namespace mparse