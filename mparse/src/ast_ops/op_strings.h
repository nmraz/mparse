#pragma once

#include "mparse/ast.h"
#include <string_view>

namespace ast_ops {

constexpr std::string_view stringify_unary_op(mparse::unary_op_type op) {
  switch (op) {
  case mparse::unary_op_type::plus:
    return "+";
  case mparse::unary_op_type::neg:
    return "-";
  default:
    return "";
  }
}

constexpr std::string_view stringify_binary_op(mparse::binary_op_type op) {
  switch (op) {
  case mparse::binary_op_type::add:
    return "+";
  case mparse::binary_op_type::sub:
    return "-";
  case mparse::binary_op_type::mult:
    return "*";
  case mparse::binary_op_type::div:
    return "/";
  case mparse::binary_op_type::pow:
    return "^";
  default:
    return "";
  }
}

} // namespace ast_ops