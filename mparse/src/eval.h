#pragma once

#include "mparse/ast/abstract_syntax_tree.h"
#include "mparse/source_range.h"
#include <stdexcept>
#include <string_view>
#include <vector>

class eval_error : public std::runtime_error {
public:
  eval_error(std::string_view what, std::vector<mparse::source_range> where);

  const std::vector<mparse::source_range>& where() const { return where_; }

private:
  std::vector<mparse::source_range> where_;
};

double eval(mparse::abstract_syntax_tree& ast);