#include "parse_error.h"

namespace mparse {

syntax_error::syntax_error(std::string_view what, std::vector<source_range> where)
  : std::runtime_error(what.data())
  , where_(std::move(where)) {
}

}  // namespace mparse