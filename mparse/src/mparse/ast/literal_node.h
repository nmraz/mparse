#pragma once

#include "mparse/ast/ast_node.h"

namespace mparse {

class literal_node : public ast_node_impl<literal_node> {
public:
  literal_node() = default;
  literal_node(double val);

  double val() const { return val_; }
  void set_val(double val);

private:
  double val_;
};

}  // namespace mparse