#include "func_util.h"

#include "ast_ops/eval/eval_error.h"
#include <sstream>

namespace ast_ops::impl {

[[noreturn]] void throw_arity_error(int expected, int provided) {
  std::ostringstream msg;
  msg << "wrong number of arguments (" << expected << " expected, " << provided << " provided)";
  throw arity_error(msg.str(), expected, provided);
}

}  // namespace ast_ops::impl