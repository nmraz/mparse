#include "parse_error.h"

namespace mparse {

syntax_error::syntax_error(std::string_view what,
                           std::vector<source_range> where,
                           std::string fixit_hint,
                           int fixit_col)
    : std::runtime_error(what.data()),
      where_(std::move(where)),
      fixit_hint_(fixit_hint),
      fixit_col_(fixit_col) {}

} // namespace mparse