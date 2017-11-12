#include "root_node.h"

#include <cassert>

namespace mparse {

source_range root_node::source_loc() const {
  return child()->source_loc();
}

}  // namespace mparse