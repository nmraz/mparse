#pragma once

#include "mparse/ast/unary_node.h"

namespace mparse {

class paren_node : public ast_node_impl<paren_node, unary_node> {
public:
  paren_node() = default;
  paren_node(ast_node* child, const source_range& loc);

  source_range source_loc() const override;
  void set_source_loc(const source_range& loc);

private:
  source_range loc_;
};

}  // namespace mparse