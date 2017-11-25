#include "eval_scope.h"

namespace ast_ops {

eval_scope::eval_scope(std::initializer_list<impl_type::value_type> ilist)
  : map_(ilist) {
}

void eval_scope::set_binding(std::string name, double value) {
  map_[std::move(name)] = value;
}

void eval_scope::remove_binding(std::string_view name) {
  auto it = map_.find(name);
  if (it != map_.end()) {
    map_.erase(it);
  }
}

std::optional<double> eval_scope::lookup(std::string_view name) const {
  auto it = map_.find(name);
  if (it != map_.end()) {
    return it->second;
  }
  return std::nullopt;
}

}  // namespace ast_ops