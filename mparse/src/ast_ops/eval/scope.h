#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>

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

}  // namespace ast_ops