#include "literal_node.h"

namespace mparse {

literal_node::literal_node(double val) {
  set_val(val);
}

void literal_node::set_val(double val) {
  val_ = val;
}

}  // namespace mparse