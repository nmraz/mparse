#pragma once

#include <cmath>
#include <vector>

namespace builtins {
  
constexpr auto e = 2.718281828459045;
constexpr auto pi = 3.141592653589793;
constexpr auto tau = 6.283185307179586;

inline double sin(double x) { return std::sin(x); }
inline double cos(double x) { return std::cos(x); }
inline double tan(double x) { return std::tan(x); }

inline double asin(double x) { return std::asin(x); }
inline double acos(double x) { return std::acos(x); }
inline double atan(double x) { return std::atan(x); }

inline double sinh(double x) { return std::sinh(x); }
inline double cosh(double x) { return std::cosh(x); }
inline double tanh(double x) { return std::tanh(x); }

inline double asinh(double x) { return std::asinh(x); }
inline double acosh(double x) { return std::acosh(x); }
inline double atanh(double x) { return std::atanh(x); }

inline double exp(double x) { return std::exp(x); }
inline double ln(double x) { return std::log(x); }
double log(double base, double val);

inline double sqrt(double x) { return std::sqrt(x); }
inline double cbrt(double x) { return std::cbrt(x); }
double nroot(double n, double val);

double min(std::vector<double> vals);
double max(std::vector<double> vals);
double avg(std::vector<double> vals);

}  // namespace builtins