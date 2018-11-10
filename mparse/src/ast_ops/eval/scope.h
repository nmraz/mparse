#pragma once

#include "ast_ops/eval/eval_error.h"
#include "ast_ops/eval/func_util.h"
#include "ast_ops/eval/types.h"
#include <initializer_list>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

namespace ast_ops {

class var_scope {
  using impl_type = std::map<std::string, number, std::less<>>;

public:
  var_scope() = default;
  var_scope(std::initializer_list<impl_type::value_type> ilist);

  void set_binding(std::string name, number val);
  void remove_binding(std::string_view name);

  void clear() { map_.clear(); }

  std::optional<number> lookup(std::string_view name) const;

private:
  impl_type map_;
};


class func_scope {
  struct func_wrapper {
    template <typename F>
    func_wrapper(F&& func) : func(wrap_function(std::forward<F>(func))) {}

    function func;
  };

  using impl_type = std::map<std::string, func_wrapper, std::less<>>;

public:
  func_scope() = default;
  func_scope(std::initializer_list<impl_type::value_type> ilist);
  
  void set_binding(std::string name, func_wrapper func);
  void remove_binding(std::string_view name);
  
  void clear() { map_.clear(); }

  const function* lookup(std::string_view name) const;

private:
  impl_type map_;
};

} // namespace ast_ops