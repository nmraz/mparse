#include "scope.h"

namespace ast_ops {

var_scope::var_scope(std::initializer_list<impl_type::value_type> ilist)
  : map_(ilist) {
}

void var_scope::set_binding(std::string name, double value) {
  map_[std::move(name)] = value;
}

void var_scope::remove_binding(std::string_view name) {
  auto it = map_.find(name);
  if (it != map_.end()) {
    map_.erase(it);
  }
}

std::optional<double> var_scope::lookup(std::string_view name) const {
  auto it = map_.find(name);
  if (it != map_.end()) {
    return it->second;
  }
  return std::nullopt;
}


func_scope::func_scope(std::initializer_list<impl_type::value_type> ilist) 
  : map_(ilist) {
}

void func_scope::remove_binding(std::string_view name) {
  auto it = map_.find(name);
  if (it != map_.end()) {
    map_.erase(it);
  }
}

const func_type* func_scope::lookup(std::string_view name) const {
  auto it = map_.find(name);
  if (it != map_.end()) {
    return &it->second;
  }
  return nullptr;
}

}  // namespace ast_ops