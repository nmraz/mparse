#pragma once

#include "mparse/source_range.h"
#include "util/span.h"
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace mparse {

class syntax_error : public std::runtime_error {
public:
  syntax_error(std::string_view what, std::vector<source_range> where,
               std::string fixit_hint = "", int fixit_col = 0);

  util::span<const source_range> where() const { return where_; }
  std::string_view fixit_hint() const { return fixit_hint_; }
  int fixit_col() const { return fixit_col_; }

private:
  std::vector<source_range> where_;
  std::string fixit_hint_;
  int fixit_col_;
};

} // namespace mparse
