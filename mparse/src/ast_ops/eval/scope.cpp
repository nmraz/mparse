#include "scope.h"

namespace ast_ops {

var_scope::var_scope(const var_scope* parent) : parent_(parent) {}

var_scope::var_scope(std::initializer_list<impl_type::value_type> ilist)
    : map_(ilist) {}

var_scope::var_scope(const var_scope* parent,
                     std::initializer_list<impl_type::value_type> ilist)
    : map_(ilist), parent_(parent) {}

void var_scope::set_binding(std::string name, number value) {
  map_[std::move(name)] = value;
}

void var_scope::remove_binding(std::string_view name) {
  auto it = map_.find(name);
  if (it != map_.end()) {
    map_.erase(it);
  }
}

std::optional<number> var_scope::lookup(std::string_view name) const {
  auto it = map_.find(name);
  if (it != map_.end()) {
    return it->second;
  }

  if (parent_) {
    return parent_->lookup(name);
  }
  return std::nullopt;
}


func_scope::func_scope(const func_scope* parent) : parent_(parent) {}

func_scope::func_scope(std::initializer_list<impl_type::value_type> ilist)
    : map_(ilist) {}

func_scope::func_scope(const func_scope* parent,
                       std::initializer_list<impl_type::value_type> ilist)
    : map_(ilist), parent_(parent) {}

void func_scope::set_binding(std::string name, func_wrapper func) {
  map_.insert_or_assign(std::move(name), std::move(func));
}

void func_scope::remove_binding(std::string_view name) {
  auto it = map_.find(name);
  if (it != map_.end()) {
    map_.erase(it);
  }
}

const function* func_scope::lookup(std::string_view name) const {
  auto it = map_.find(name);
  if (it != map_.end()) {
    return &it->second.func;
  }

  if (parent_) {
    return parent_->lookup(name);
  }
  return nullptr;
}

} // namespace ast_ops