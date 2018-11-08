#pragma once

#include "util/span.h"
#include <complex>
#include <functional>
#include <vector>

namespace ast_ops {

using number = std::complex<double>;

using func_args = util::span<const number>;
using real_func_args = util::span<const double>;

using function = std::function<number(func_args)>;
using real_function = std::function<number(real_func_args)>;

} // namespace ast_ops