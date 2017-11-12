#include "paren_node.h"
#include <cassert>

namespace mparse {

paren_node::paren_node(ast_node* child, const source_range& loc) {
  set_child(child);
  set_source_loc(loc);
}

void paren_node::set_source_loc(const source_range& loc) {
  loc_ = loc;
}

source_range paren_node::source_loc() const {
  assert(
    loc_.from() < child()->source_loc().from()
    && loc_.to() > child()->source_loc().to()
    && "Subexpression should be fully contained by parentheses"
  );
  return loc_;
}

}  // namespace mparse