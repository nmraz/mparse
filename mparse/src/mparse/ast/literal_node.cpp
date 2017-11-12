#include "literal_node.h"

namespace mparse {

literal_node::literal_node(double val, const source_range& loc) {
  set_val(val);
  set_source_loc(loc);
}

void literal_node::set_val(double val) {
  val_ = val;
}

void literal_node::set_source_loc(const source_range& loc) {
  loc_ = loc;
}

}  // namespace mparse