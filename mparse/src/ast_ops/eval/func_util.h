#pragma once

#include "ast_ops/eval/types.h"
#include "util/meta.h"
#include <algorithm>
#include <functional>
#include <type_traits>
#include <vector>

namespace ast_ops {
namespace impl {

void check_arity(std::size_t expected, std::size_t provided);

void throw_if_nonreal(std::vector<std::size_t> nonreal_args);
void check_real(func_args args);

template <typename F>
struct get_memfun_args;

template <typename C, typename R, typename... Args>
struct get_memfun_args<R (C::*)(Args...)> : util::type_list<Args...> {};

template <typename C, typename R, typename... Args>
struct get_memfun_args<R (C::*)(Args...) const> : util::type_list<Args...> {};

template <typename F>
struct get_args : get_memfun_args<decltype(&F::operator())> {};

template <typename R, typename... Args>
struct get_args<R (*)(Args...)> : util::type_list<Args...> {};


template <typename T>
struct arg_checker {
  static_assert(util::always_false<T>,
                "Functions must take only numbers or doubles");
};

template <>
struct arg_checker<number> {
  static bool check(const number&) { return true; }
  static number convert(const number& x) { return x; }
};

template <>
struct arg_checker<double> {
  static bool check(const number& x) { return x.imag() == 0; }
  static double convert(const number& x) { return x.real(); }
};


template <std::size_t... I, typename... Args>
void check_types(func_args args, std::index_sequence<I...>,
                 util::type_list<Args...>) {
  std::vector<std::size_t> nonreal_args;
  ((!arg_checker<Args>::check(args[I]) ? nonreal_args.push_back(I) : (void) 0),
   ...);
  throw_if_nonreal(std::move(nonreal_args));
}

template <typename F, std::size_t... I, typename... Args>
number invoke_helper(F& func, func_args args, std::index_sequence<I...> idx,
                     util::type_list<Args...> ts) {
  check_arity(sizeof...(Args), args.size());
  check_types(args, idx, ts);
  return func(arg_checker<Args>::convert(args[I])...);
}

} // namespace impl


template <typename F>
function wrap_function(F&& func) {
  if constexpr (std::is_convertible_v<F&&, function>) {
    return std::forward<F>(func);
  } else if constexpr (std::is_convertible_v<F&&, real_function>) {
    return [func = std::forward<F>(func)](func_args args) {
      impl::check_real(args);

      std::vector<double> real_args;
      std::transform(args.begin(), args.end(), std::back_inserter(real_args),
                     [](const number& x) { return x.real(); });

      return func(real_args);
    };
  } else {
    return [func = std::forward<F>(func)](func_args args) {
      using arg_types = impl::get_args<std::decay_t<F>>;
      return impl::invoke_helper(func, args, typename arg_types::seq{},
                                 arg_types{});
    };
  }
}

} // namespace ast_ops