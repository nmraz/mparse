#pragma once

#include <vector>

namespace builtins {
  
constexpr auto e = 2.718281828459045;
constexpr auto pi = 3.141592653589793;
constexpr auto tau = 6.283185307179586;


double mod(double a, double b);

double sin(double x);
double cos(double x);
double tan(double x);

double asin(double x);
double acos(double x);
double atan(double x);

double sinh(double x);
double cosh(double x);
double tanh(double x);

double asinh(double x);
double acosh(double x);
double atanh(double x);

double exp(double x);
double ln(double x);
double log(double base, double val);

double sqrt(double x);
double cbrt(double x);
double nroot(double n, double val);

double min(std::vector<double> vals);
double max(std::vector<double> vals);
double avg(std::vector<double> vals);

}  // namespace builtins