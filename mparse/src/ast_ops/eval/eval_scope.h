#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace ast_ops {

class eval_scope {
  using impl_type = std::map<std::string, double, std::less<>>;

public:
  using iterator = impl_type::iterator;
  using const_iterator = impl_type::const_iterator;
  using reverse_iterator = impl_type::reverse_iterator;
  using const_reverse_iterator = impl_type::const_reverse_iterator;

  eval_scope() = default;
  eval_scope(std::initializer_list<impl_type::value_type> ilist);

  iterator begin() { return map_.begin(); }
  const_iterator begin() const { return map_.begin(); }
  const_iterator cbegin() const { return map_.cbegin(); }

  iterator end() { return map_.end(); }
  const_iterator end() const { return map_.end(); }
  const_iterator cend() const { return map_.cend(); }

  reverse_iterator rbegin() { return map_.rbegin(); }
  const_reverse_iterator rbegin() const { return map_.rbegin(); }
  const_reverse_iterator crbegin() const { return map_.crbegin(); }

  reverse_iterator rend() { return map_.rend(); }
  const_reverse_iterator rend() const { return map_.rend(); }
  const_reverse_iterator crend() const { return map_.crend(); }

  void set_binding(std::string name, double val);
  void remove_binding(std::string_view name);

  void clear() { map_.clear(); }

  std::optional<double> lookup(std::string_view name) const;

private:
  impl_type map_;
};

}  // namespace ast_ops