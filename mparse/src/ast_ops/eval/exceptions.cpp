#include "exceptions.h"

namespace ast_ops {

eval_error::eval_error(std::string_view what, eval_errc code, const mparse::ast_node* node)
  : std::runtime_error(what.data())
  , code_(code)
  , node_(node) {
}

}  // namespace ast_ops