#pragma once

#include "mparse/ast/ast_node.h"
#include <type_traits>

namespace mparse {

template<typename Der, typename Base = ast_node>
class ast_node_impl : public Base {
public:
  static_assert(std::is_base_of_v<ast_node, Base>, "AST nodes must derive from ast_node");

  void apply_visitor(ast_visitor& vis) override {
    Base::apply_visitor(vis);  // visit bases first
    vis.visit(static_cast<Der&>(*this));
  }

  void apply_visitor(const_ast_visitor& vis) const override {
    Base::apply_visitor(vis);  // visit bases first
    vis.visit(static_cast<const Der&>(*this));
  }
};

}  // namespace mparse