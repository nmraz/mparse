#pragma once

#include "mparse/ast/ast_node_impl.h"

namespace mparse {

class literal_node : public ast_node_impl<literal_node> {
public:
  constexpr literal_node() = default;
  constexpr explicit literal_node(double val) : val_(val) {}

  constexpr double val() const { return val_; }
  constexpr void set_val(double val) { val_ = val; }

private:
  double val_ = 0;
};

} // namespace mparse