#pragma once

#include "mparse/ast/ast_node_impl.h"
#include <string>
#include <vector>

namespace mparse {

class func_node : public ast_node_impl<func_node> {
public:
  using child_list = std::vector<ast_node_ptr>;

  func_node() = default;
  func_node(std::string name, child_list children);

  std::string name() const { return name_; }
  void set_name(std::string name);

  const child_list& children() const { return children_; }
  child_list& children() { return children_; }

private:
  std::string name_;
  child_list children_;
};

}  // namespace mparse