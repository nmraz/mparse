#pragma once

#include "mparse/source_range.h"
#include <initializer_list>
#include <string_view>

void print_loc(std::string_view source, mparse::source_range loc);
void print_locs(std::string_view source, std::initializer_list<mparse::source_range> locs);