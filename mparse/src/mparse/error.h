#pragma once

#include "mparse/source_range.h"
#include <stdexcept>

namespace mparse {

class syntax_error : public std::runtime_error {
public:
  syntax_error(std::string_view what, source_range where);

  source_range where() const { return where_; }

private:
  source_range where_;
};

}  // namespace mparse
