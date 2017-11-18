#pragma once

#include "mparse/source_range.h"
#include <initializer_list>
#include <string_view>

void print_loc(mparse::source_range loc, std::string_view source);
void print_locs(std::initializer_list<mparse::source_range> locs, std::string_view source);