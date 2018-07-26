#pragma once

#include <complex>
#include <functional>
#include <vector>

namespace ast_ops {

using number = std::complex<double>;
using function = std::function<number(std::vector<number>)>;

}  // namespace ast_ops