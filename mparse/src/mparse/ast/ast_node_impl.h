#pragma once

#include "mparse/ast/ast_node.h"
#include "util/meta.h"

namespace mparse {
namespace impl {

template <typename V, typename T, typename = void>
constexpr bool has_visit_overload = false;

template <typename V, typename T>
constexpr bool has_visit_overload<
    V, T, std::void_t<decltype(static_cast<void (V::*)(T&)>(&V::visit))>> =
    true;

template <typename T, typename = void>
struct get_derived_types {
  using type = util::type_list<>;
};

template <typename T>
struct get_derived_types<T, std::void_t<typename T::derived_types>> {
  using type = typename T::child_types;
};

template <typename T>
using get_derived_types_t = typename get_derived_types<T>::type;

template <typename T, typename D>
struct flatten_derived_types;

template <typename T>
using get_flattened_derived_types_t =
    typename flatten_derived_types<T, get_derived_types_t<T>>::type;

template <typename T, typename... Ds>
struct flatten_derived_types<T, util::type_list<Ds...>> {
  using type = util::type_list_cat_t<util::type_list<T, Ds...>,
                                     get_flattened_derived_types_t<Ds>...>;
};

} // namespace impl

template <typename Der, typename Base>
class ast_node_impl : public Base {
public:
  static_assert(std::is_base_of_v<ast_node, Base>,
                "AST nodes must derive from ast_node");
  static_assert(impl::has_visit_overload<ast_visitor, Der>,
                "Missing ast_visitor overload");

  constexpr ast_node_impl() { this->id_ = get_id(); }

  void apply_visitor(ast_visitor& vis) override {
    Base::apply_visitor(vis); // visit bases first
    vis.visit(static_cast<Der&>(*this));
  }

  void apply_visitor(const_ast_visitor& vis) const override {
    Base::apply_visitor(vis); // visit bases first
    vis.visit(static_cast<const Der&>(*this));
  }


  bool has_id(const void* id) const override {
    return id == get_id() || Base::has_id(id);
  }

  static bool classof(const ast_node& node) {
    return do_classof(node.id_, impl::get_flattened_derived_types_t<Der>{});
  }

private:
  template <typename D, typename B>
  friend class ast_node_impl;

  static constexpr const void* get_id() { return &id_dummy; }

  template <typename D, typename... Ds>
  static bool do_classof(const void* id, util::type_list<D, Ds...>) {
    if (id == D::get_id()) {
      return true;
    }

    if constexpr (sizeof...(Ds) > 0) {
      return do_classof(id, util::type_list<Ds...>{});
    } else {
      return true;
    }
  }

  static inline char id_dummy;
};

} // namespace mparse