#include "id_node.h"

namespace mparse {

id_node::id_node(std::string name) {
  set_name(std::move(name));
}

void id_node::set_name(std::string name) {
  name_ = std::move(name);
}

} // namespace mparse