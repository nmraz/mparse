#pragma once

#include "mparse/ast.h"
#include "mparse/source_map.h"
#include <iosfwd>

namespace ast_ops {

void dump_ast(const mparse::ast_node& node, const mparse::source_map* smap,
              std::ostream& stream);
void dump_ast(const mparse::ast_node& node,
              const mparse::source_map* smap = nullptr);

} // namespace ast_ops