#pragma once

#include "mparse/ast/operator_nodes.h"

const char* stringify_unary_op(mparse::unary_op_type op);
const char* stringify_binary_op(mparse::binary_op_type op);