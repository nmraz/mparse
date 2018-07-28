#pragma once

#include "mparse/ast/ast_node.h"
#include "mparse/ast/ast_visitor.h"
#include <type_traits>

namespace mparse {
namespace impl {

template<typename T>
struct cast_visitor : ast_visitor {
  void visit(T& node) override {
    result = &node;
  }

  T* result = nullptr;
};

template<typename T, typename U>
struct cast_helper {
  static_assert(std::is_base_of_v<ast_node, T>, "ast_node_cast can only cast AST nodes");
  static_assert(std::is_base_of_v<ast_node, U>, "ast_node_cast can only cast AST nodes");

  static T* do_cast(U* node) {
    if constexpr (std::is_base_of_v<T, U>) {  // upcast
      return node;
    } else {  // downcast
      static_assert(std::is_base_of_v<U, T>, "Crosscasts are not supported");

      if (!node) {
        return nullptr;
      }

      cast_visitor<T> vis;
      node->apply_visitor(vis);
      return vis.result;
    }
  }
};

}  // namespace impl

template<typename T, typename U>
inline T* ast_node_cast(U* node) {
  return static_cast<T*>(impl::cast_helper<std::remove_cv_t<T>, U>::do_cast(node));
}

template<typename T, typename U>
inline T* ast_node_cast(const U* node) {
  static_assert(std::is_const_v<T>, "ast_node_cast cannot remove const qualification");
  return ast_node_cast<T>(const_cast<U*>(node));
}


template<typename T, typename U>
inline node_ptr<T> static_ast_node_ptr_cast(const node_ptr<U>& node) {
  return std::static_pointer_cast<T>(node);
}

template<typename T, typename U>
inline node_ptr<T> ast_node_ptr_cast(const node_ptr<U>& node) {
  if (auto cast = ast_node_cast<T>(node.get())) {
    return node_ptr<T>(node, cast);
  }
  return nullptr;
}

}  // namespace mparse