#pragma once

#include "mparse/ast/ast_node.h"
#include <type_traits>

namespace mparse {
namespace impl {

template <typename V, typename T, typename = void>
constexpr bool has_visit_overload = false;

template <typename V, typename T>
constexpr bool has_visit_overload<
    V, T, std::void_t<decltype(static_cast<void (V::*)(T&)>(&V::visit))>> =
    true;

} // namespace impl

template <typename Der, typename Base = ast_node>
class ast_node_impl : public Base {
public:
  static_assert(std::is_base_of_v<ast_node, Base>,
                "AST nodes must derive from ast_node");
  static_assert(impl::has_visit_overload<ast_visitor, Der>,
                "Missing ast_visitor overload");

  void apply_visitor(ast_visitor& vis) override {
    Base::apply_visitor(vis); // visit bases first
    vis.visit(static_cast<Der&>(*this));
  }

  void apply_visitor(const_ast_visitor& vis) const override {
    Base::apply_visitor(vis); // visit bases first
    vis.visit(static_cast<const Der&>(*this));
  }
};

} // namespace mparse