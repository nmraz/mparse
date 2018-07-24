#include "func_node.h"

namespace mparse {

func_node::func_node(std::string name, child_list children)
  : name_(std::move(name))
  , children_(std::move(children)) {
}

void func_node::set_name(std::string name) {
  name_ = name;
}

}  // namespace mparse