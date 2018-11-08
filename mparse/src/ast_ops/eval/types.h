#pragma once

#include "util/span.h"
#include <complex>
#include <functional>
#include <vector>

namespace ast_ops {

using number = std::complex<double>;
using function = std::function<number(util::span<const number>)>;
using real_function = std::function<number(util::span<const double>)>;

} // namespace ast_ops