#pragma once

#include "mparse/ast/unary_node.h"

namespace mparse {

class root_node : public ast_node_impl<root_node, unary_node> {
public:
  source_range source_loc() const;
};

}  // namespace mparse