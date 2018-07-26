#pragma once

#include <functional>
#include <type_traits>
#include <vector>

namespace ast_ops {
namespace impl {

[[noreturn]] void throw_arity_error(int expected, int provided);

template<typename F>
struct get_memfun_arity;

template<typename C, typename R, typename... Args>
struct get_memfun_arity<R(C::*)(Args...)>
  : std::integral_constant<std::size_t, sizeof...(Args)> {
};

template<typename C, typename R, typename... Args>
struct get_memfun_arity<R(C::*)(Args...) const>
  : std::integral_constant<std::size_t, sizeof...(Args)> {
};


template<typename F>
struct get_arity
  : get_memfun_arity<decltype(&F::operator())> {
};

template<typename R, typename... Args>
struct get_arity<R(*)(Args...)>
  : std::integral_constant<std::size_t, sizeof...(Args)> {
};

template<typename F>
constexpr std::size_t get_arity_v = get_arity<std::decay_t<F>>::value;

template<typename F, std::size_t... I>
double invoke_helper(F& func, const std::vector<double>& args, std::index_sequence<I...>) {
  return func(args[I]...);
}

}  // namespace impl  


using function = std::function<double(std::vector<double>)>;

template<typename F>
function wrap_function(F&& func) {
  if constexpr (std::is_convertible_v<F&&, function>) {
    return std::forward<F>(func);
  } else {
    return [func = std::forward<F>(func)] (std::vector<double> args) {
      constexpr auto arity = static_cast<int>(impl::get_arity_v<F>);
      if (args.size() != arity) {
        impl::throw_arity_error(arity, static_cast<int>(args.size()));
      }
      return impl::invoke_helper(func, args, std::make_index_sequence<arity>());
    };
  }
}

}  // namespace ast_ops