#pragma once

#include "mparse/ast.h"
#include "mparse/source_map.h"
#include <string>

namespace ast_ops {

std::string pretty_print(const mparse::ast_node& ast,
                         mparse::source_map* smap = nullptr);

} // namespace ast_ops