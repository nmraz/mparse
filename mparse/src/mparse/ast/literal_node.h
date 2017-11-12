#pragma once

#include "mparse/ast/ast_node.h"

namespace mparse {

class literal_node : public ast_node_impl<literal_node> {
public:
  literal_node() = default;
  literal_node(double val, const source_range& loc);

  double val() const { return val_; }
  void set_val(double val);

  source_range source_loc() const override { return  loc_; }
  void set_source_loc(const source_range& loc);

private:
  double val_;
  source_range loc_;
};

}  // namespace mparse