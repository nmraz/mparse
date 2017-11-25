#pragma once

#include "mparse/ast/operator_nodes.h"

namespace ast_ops::matching {

constexpr bool is_commutative(mparse::binary_op_type op) {
  switch (op) {
  case mparse::binary_op_type::add:
  case mparse::binary_op_type::mult:
    return true;
  default:
    return false;
  }
}

}  // namespace ast_ops::matching