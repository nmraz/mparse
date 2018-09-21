#pragma once

#include <complex>
#include <functional>
#include <vector>

namespace ast_ops {

using number = std::complex<double>;
using function = std::function<number(std::vector<number>)>;
using real_function = std::function<number(std::vector<double>)>;

} // namespace ast_ops