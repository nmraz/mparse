#pragma once

#include "mparse/ast/ast_node.h"
#include <string>

namespace ast_ops {

std::string pretty_print(const mparse::ast_node* ast);

}  // namespace ast_ops