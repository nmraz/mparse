#pragma once

#include "ast_ops/eval/scope.h"
#include "ast_ops/eval/types.h"
#include <vector>

namespace ast_ops {
namespace builtins {

constexpr auto e = 2.718281828459045;
constexpr auto pi = 3.141592653589793;
constexpr auto tau = 6.283185307179586;
constexpr auto i = number(0, 1);


number sin(number x);
number cos(number x);
number tan(number x);

number asin(number x);
number acos(number x);
number atan(number x);

number sinh(number x);
number cosh(number x);
number tanh(number x);

number asinh(number x);
number acosh(number x);
number atanh(number x);

number exp(number x);
number ln(number x);
number log(number base, number val);

number sqrt(number x);
double cbrt(double x);
double nroot(double n, double val);

double re(number x);
double im(number x);
double arg(number x);
number conj(number x);

double floor(double x);
double ceil(double x);
double round(double x);

double mod(double a, double b);

double min(util::span<const double> vals);
double max(util::span<const double> vals);
number avg(util::span<const number> vals);

} // namespace builtins

var_scope builtin_var_scope();
func_scope builtin_func_scope();

} // namespace ast_ops