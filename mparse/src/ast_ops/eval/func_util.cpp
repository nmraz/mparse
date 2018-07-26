#include "func_util.h"

#include "ast_ops/eval/eval_error.h"
#include <sstream>

namespace ast_ops::impl {

[[noreturn]] void throw_arity_error(int expected, int provided) {
  std::ostringstream msg;
  msg << "wrong number of arguments (" << expected << " expected, " << provided << " provided)";
  throw arity_error(msg.str(), expected, provided);
}

void check_real(const std::vector<number> args) {
  std::vector<int> nonreal_args;
  for (int i = 0; i < args.size(); i++) {
    if (args[i].imag() != 0) {
      nonreal_args.push_back(i);
    }
  }

  if (!nonreal_args.empty()) {
    std::ostringstream msg;
    msg << (nonreal_args.size() > 1 ? "argument" : "arguments")
      << " must be real";
    throw func_arg_error(msg.str(), std::move(nonreal_args));
  }
}

}  // namespace ast_ops::impl