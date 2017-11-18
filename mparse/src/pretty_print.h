#pragma once

#include "mparse/ast/abstract_syntax_tree.h"
#include <string>

std::string pretty_print(const mparse::ast_node* ast);