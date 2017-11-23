#include "op_strings.h"

namespace ast_ops {

const char* stringify_unary_op(mparse::unary_op_type type) {
  switch (type) {
  case mparse::unary_op_type::plus:
    return "+";
  case mparse::unary_op_type::neg:
    return "-";
  default:
    return "";
  }
}

const char* stringify_binary_op(mparse::binary_op_type type) {
  switch (type) {
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

}  // namespace ast_ops