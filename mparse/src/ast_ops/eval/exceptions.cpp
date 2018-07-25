#include "exceptions.h"

namespace ast_ops {

eval_error::eval_error(std::string_view what, eval_errc code, const mparse::ast_node* node)
  : std::runtime_error(what.data())
  , code_(code)
  , node_(node) {
}

arity_error::arity_error(std::string_view what, int expected, int provided)
  : std::runtime_error(what.data())
  , expected_(expected)
  , provided_(provided) {
}

}  // namespace ast_ops