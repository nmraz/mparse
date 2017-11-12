#include "error.h"

namespace mparse {

syntax_error::syntax_error(std::string_view what, source_range where)
  : std::runtime_error(what.data())
  , where_(where) {
}

}  // namespace mparse