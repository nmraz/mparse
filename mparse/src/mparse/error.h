#pragma once

#include "mparse/source_range.h"
#include "util/span.h"
#include <stdexcept>
#include <vector>

namespace mparse {

class syntax_error : public std::runtime_error {
public:
  syntax_error(std::string_view what, std::vector<source_range> where);

  util::span<const source_range> where() const { return where_; }

private:
  std::vector<source_range> where_;
};

}  // namespace mparse
