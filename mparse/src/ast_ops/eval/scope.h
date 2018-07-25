#pragma once

#include "ast_ops/eval/exceptions.h"
#include <functional>
#include <initializer_list>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace ast_ops {

class var_scope {
  using impl_type = std::map<std::string, double, std::less<>>;

public:
  var_scope() = default;
  var_scope(std::initializer_list<impl_type::value_type> ilist);

  void set_binding(std::string name, double val);
  void remove_binding(std::string_view name);

  void clear() { map_.clear(); }

  std::optional<double> lookup(std::string_view name) const;

private:
  impl_type map_;
};


namespace impl {

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


template<typename F, std::size_t... I>
double invoke_helper(F& func, const std::vector<double>& args, std::index_sequence<I...>) {
  return func(args[I]...);
}

[[noreturn]] void throw_arity_error(int expected, int provided);

}  // namespace impl


class func_scope {
public:
  using func_type = std::function<double(std::vector<double>)>;

private:
  using impl_type = std::map<std::string, func_type, std::less<>>;

public:
  func_scope() = default;
  func_scope(std::initializer_list<impl_type::value_type> ilist);
  
  void set_binding(std::string name, func_type func);
  template<typename F, typename = std::enable_if_t<!std::is_convertible_v<F&&, func_type>>>
  void set_binding(std::string name, F&& func);
  
  void remove_binding(std::string_view name);
  void clear() { map_.clear(); }

  const func_type* lookup(std::string_view name) const;

private:
  impl_type map_;
};


template<typename F, typename>
void func_scope::set_binding(std::string name, F&& func) {
  using namespace std::literals;

  // MSVC constexpr capture bug workaround - replace with variable templates when fixed.
  constexpr impl::get_arity<std::decay_t<F>> arity_obj;

  set_binding(std::move(name), [f = std::forward<F>(func), arity_obj](std::vector<double> args) {
    constexpr int arity = arity_obj.value;
    if (args.size() != arity) {
      impl::throw_arity_error(arity, static_cast<int>(args.size()));
    }
 
    return impl::invoke_helper(f, args, std::make_index_sequence<arity>{});
  });
}

}  // namespace ast_ops