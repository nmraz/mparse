#include "eval_error.h"

#include <algorithm>

namespace ast_ops {

eval_error::eval_error(std::string_view what,
                       eval_errc code,
                       const mparse::ast_node* node)
    : std::runtime_error(what.data()), code_(code), node_(node) {}

arity_error::arity_error(std::string_view what, int expected, int provided)
    : std::runtime_error(what.data()),
      expected_(expected),
      provided_(provided) {}

func_arg_error::func_arg_error(std::string_view what, std::vector<int> indices)
    : std::runtime_error(what.data()), indices_(std::move(indices)) {
  std::sort(indices_.begin(), indices_.end());
}

} // namespace ast_ops