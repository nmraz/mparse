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

template <typename T, typename D>
struct flatten_derived_types;

template <typename T>
using get_flattened_derived_types_t =
    typename flatten_derived_types<T, typename T::derived_types>::type;

template <typename T, typename... Ds>
struct flatten_derived_types<T, util::type_list<Ds...>> {
  using type = util::type_list_cat_t<util::type_list<T>,
                                     get_flattened_derived_types_t<Ds>...>;
};

template <typename Der, typename Base>
constexpr bool is_listed_as_derived =
    util::type_list_count_v<Der, typename Base::derived_types>;

} // namespace impl

template <typename Der, typename Base>
class ast_node_impl : public Base {
public:
  static_assert(std::is_base_of_v<ast_node, Base>,
                "AST nodes must derive from ast_node");
  static_assert(impl::has_visit_overload<ast_visitor, Der>,
                "Missing ast_visitor overload");
  static_assert(impl::is_listed_as_derived<Der, Base>,
                "Type not listed in Base::derived_types");

  using derived_types = util::type_list<>;

  constexpr ast_node_impl() { this->id_ = get_id(); }

  void apply_visitor(ast_visitor& vis) override {
    Base::apply_visitor(vis); // visit bases first
    vis.visit(static_cast<Der&>(*this));
  }

  void apply_visitor(const_ast_visitor& vis) const override {
    Base::apply_visitor(vis); // visit bases first
    vis.visit(static_cast<const Der&>(*this));
  }


private:
  template <typename D, typename B>
  friend class ast_node_impl;

  template <typename T, typename U>
  friend T* ast_node_cast(U* node);

  static constexpr const void* get_id() { return &id_dummy; }

  static bool classof(const ast_node& node) {
    return do_classof(node.id_, impl::get_flattened_derived_types_t<Der>{});
  }

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