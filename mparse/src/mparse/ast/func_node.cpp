#include "func_node.h"

namespace mparse {

func_node::func_node(std::string name, arg_list args)
  : name_(std::move(name))
  , args_(std::move(args)) {
}

void func_node::set_name(std::string name) {
  name_ = name;
}

}  // namespace mparse