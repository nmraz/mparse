#pragma once

#include "ast_ops/eval/types.h"
#include <vector>

namespace builtins {
  
constexpr auto e = 2.718281828459045;
constexpr auto pi = 3.141592653589793;
constexpr auto tau = 6.283185307179586;
constexpr auto i = ast_ops::number(0, 1);


double mod(double a, double b);

ast_ops::number sin(ast_ops::number x);
ast_ops::number cos(ast_ops::number x);
ast_ops::number tan(ast_ops::number x);

ast_ops::number asin(ast_ops::number x);
ast_ops::number acos(ast_ops::number x);
ast_ops::number atan(ast_ops::number x);

ast_ops::number sinh(ast_ops::number x);
ast_ops::number cosh(ast_ops::number x);
ast_ops::number tanh(ast_ops::number x);

ast_ops::number asinh(ast_ops::number x);
ast_ops::number acosh(ast_ops::number x);
ast_ops::number atanh(ast_ops::number x);

ast_ops::number exp(ast_ops::number x);
ast_ops::number ln(ast_ops::number x);
ast_ops::number log(ast_ops::number base, ast_ops::number val);

ast_ops::number sqrt(ast_ops::number x);
double cbrt(double x);
double nroot(double n, double val);

double re(ast_ops::number x);
double im(ast_ops::number x);
double arg(ast_ops::number x);
ast_ops::number conj(ast_ops::number x);

double floor(double x);
double ceil(double x);
double round(double x);

double min(std::vector<double> vals);
double max(std::vector<double> vals);
ast_ops::number avg(std::vector<ast_ops::number> vals);

}  // namespace builtins