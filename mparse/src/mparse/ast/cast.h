#pragma once

#include "mparse/ast/ast_node.h"
#include <type_traits>

namespace mparse {

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

} // namespace mparse