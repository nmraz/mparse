#pragma once

#include "mparse/ast/ast_node_impl.h"
#include <string>
#include <vector>

namespace mparse {

class func_node : public ast_node_impl<func_node> {
public:
  using arg_list = std::vector<ast_node_ptr>;

  func_node() = default;
  func_node(std::string name, arg_list args);

  std::string name() const { return name_; }
  void set_name(std::string name);

  const arg_list& args() const { return args_; }
  arg_list& args() { return args_; }

private:
  std::string name_;
  arg_list args_;
};

}  // namespace mparse