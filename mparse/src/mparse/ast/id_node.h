#pragma once

#include "mparse/ast/ast_node_impl.h"
#include <string>

namespace mparse {

class id_node : public ast_node_impl<id_node> {
public:
  id_node() = default;
  id_node(std::string name);

  std::string name() const { return name_; }
  void set_name(std::string name);

private:
  std::string name_;
};

} // namespace mparse