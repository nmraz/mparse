#pragma once

#include "mparse/ast/ast_visitor.h"
#include "util/meta.h"
#include <memory>

namespace mparse {

class ast_node;

class unary_node;
class paren_node;
class abs_node;
class unary_op_node;
class binary_op_node;
class func_node;
class literal_node;
class id_node;


template <typename T>
using node_ptr = std::shared_ptr<T>;
using ast_node_ptr = node_ptr<ast_node>;

template <typename Node, typename... Args>
node_ptr<Node> make_ast_node(Args&&... args) {
  static_assert(std::is_base_of_v<ast_node, Node>,
                "make_ast_node can only be used for AST nodes");
  return std::make_shared<Node>(std::forward<Args>(args)...);
}


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

template <typename Der, typename Base = ast_node>
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

  template <typename N>
  static bool classof(const N& node) {
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
      return false;
    }
  }

  static inline char id_dummy;
};


template <typename T, typename U>
T* ast_node_cast(U* node) {
  static_assert(std::is_base_of_v<ast_node, T>,
                "ast_node_cast can only cast AST nodes");
  static_assert(std::is_base_of_v<ast_node, U>,
                "ast_node_cast can only cast AST nodes");

  if constexpr (std::is_base_of_v<T, U>) { // upcast
    return node;
  } else { // downcast
    static_assert(std::is_base_of_v<U, T>, "Crosscasts are not supported");

    if (!node || !T::classof(*node)) {
      return nullptr;
    }

    return static_cast<T*>(node);
  }
}


template <typename T, typename U,
          typename = std::enable_if_t<std::is_base_of_v<ast_node, U>>>
inline node_ptr<T> static_ast_node_ptr_cast(const node_ptr<U>& node) {
  return std::static_pointer_cast<T>(node);
}

template <typename T, typename U,
          typename = std::enable_if_t<std::is_base_of_v<ast_node, U>>>
inline node_ptr<T> ast_node_ptr_cast(const node_ptr<U>& node) {
  if (auto cast = ast_node_cast<T>(node.get())) {
    return node_ptr<T>(node, cast);
  }
  return nullptr;
}


namespace impl {

template <typename V, typename N, template <typename> typename AddCv>
struct visit_helper {
  static void do_apply_visitor(V&, N&, util::type_list<>) {}

  template <typename D, typename... Ds>
  static void do_apply_visitor(V& vis, N& node, util::type_list<D, Ds...>) {
    if (auto* der_node = ast_node_cast<AddCv<D>>(&node)) {
      vis(*der_node);
      do_apply_visitor(vis, node, typename D::derived_types{});
    } else {
      do_apply_visitor(vis, node, util::type_list<Ds...>{});
    }
  }

  static void apply_visitor(V& vis, N& node) {
    vis(node);
    do_apply_visitor(vis, node, typename N::derived_types{});
  }
};

} // namespace impl

template <typename D>
struct ast_visitor1 {};

template <typename D>
struct const_ast_visitor1 {};

template <typename V>
void apply_visitor(ast_visitor1<V>& vis, ast_node& node) {
  impl::visit_helper<V, ast_node, util::identity>::apply_visitor(
      static_cast<V&>(vis), node);
}

template <typename V>
void apply_visitor(const_ast_visitor1<V>& vis, const ast_node& node) {
  impl::visit_helper<V, const ast_node, std::add_const_t>::apply_visitor(
      static_cast<V&>(vis), node);
}

} // namespace mparse