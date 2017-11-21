#pragma once

#include "mparse/ast/ast_node.h"
#include "mparse/source_map.h"

void dump_ast(const mparse::ast_node* node, const mparse::source_map& smap);