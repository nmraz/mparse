#include "scope.h"

#include <sstream>

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


namespace impl {

[[noreturn]] void throw_arity_error(int expected, int provided) {
  std::ostringstream msg;
  msg << "wrong number of arguments; " << expected << " expected, " << provided << " provided";
  throw arity_error(msg.str(), expected, provided);
}

}  // namespace impl


func_scope::func_scope(std::initializer_list<impl_type::value_type> ilist) 
  : map_(ilist) {
}

void func_scope::set_binding(std::string name, func_type func) {
  map_[std::move(name)] = std::move(func);
}

void func_scope::remove_binding(std::string_view name) {
  auto it = map_.find(name);
  if (it != map_.end()) {
    map_.erase(it);
  }
}

const func_scope::func_type* func_scope::lookup(std::string_view name) const {
  auto it = map_.find(name);
  if (it != map_.end()) {
    return &it->second;
  }
  return nullptr;
}

}  // namespace ast_ops