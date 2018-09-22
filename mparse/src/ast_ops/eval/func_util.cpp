#include "func_util.h"

#include "ast_ops/eval/eval_error.h"
#include <sstream>

namespace ast_ops::impl {

void check_arity(std::size_t expected, std::size_t provided) {
  if (expected != provided) {
    std::ostringstream msg;
    msg << "wrong number of arguments (" << expected << " expected, "
        << provided << " provided)";
    throw arity_error(msg.str(), expected, provided);
  }
}


void throw_if_nonreal(std::vector<std::size_t> nonreal_args) {
  if (!nonreal_args.empty()) {
    std::ostringstream msg;
    msg << (nonreal_args.size() > 1 ? "arguments" : "argument")
        << " must be real";
    throw func_arg_error(msg.str(), std::move(nonreal_args));
  }
}

void check_real(const std::vector<number>& args) {
  std::vector<std::size_t> nonreal_args;
  for (std::size_t i = 0; i < args.size(); i++) {
    if (args[i].imag() != 0) {
      nonreal_args.push_back(i);
    }
  }
  throw_if_nonreal(std::move(nonreal_args));
}

} // namespace ast_ops::impl