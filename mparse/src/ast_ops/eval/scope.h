#pragma once

#include "ast_ops/eval/eval_error.h"
#include "ast_ops/eval/func_util.h"
#include <initializer_list>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

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


class func_scope {
  using impl_type = std::map<std::string, func_type, std::less<>>;

public:
  func_scope() = default;
  func_scope(std::initializer_list<impl_type::value_type> ilist);
 
  template<typename F>
  void set_binding(std::string name, F&& func);
  
  void remove_binding(std::string_view name);
  void clear() { map_.clear(); }

  const func_type* lookup(std::string_view name) const;

private:
  impl_type map_;
};


template<typename F>
void func_scope::set_binding(std::string name, F&& func) {
  map_[std::move(name)] = wrap_function(std::forward<F>(func));
}

}  // namespace ast_ops