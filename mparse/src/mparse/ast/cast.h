#pragma once

#include "mparse/ast/ast_node.h"
#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/util.h"
#include <type_traits>

namespace mparse {
namespace impl {

template<typename T, template<typename> AddCv>
struct cast_visitor : ast_visitor_cv<AddCv> {
  void visit(AddCv<T>& node) override {
    result = &to;
  }

  AddCv<T>* result = nullptr;
};

template<typename T, typename U, template<typename> AddCv>
struct cast_helper {
  static_assert(std::is_base_of_v<ast_node, T>, "ast_node_cast can only be used for AST nodes");

  static AddCv<T>* do_cast(AddCv<U>* node) {
    if constexpr (std::is_base_of_v<T, U>) {  // upcast
      return node;
    } else {  // downcast
      static_assert(std::is_base_of_v<U, T>, "Crosscasts are not supported");

      if (!node) {
        return nullptr;
      }

      cast_visitor<T, AddCv> vis;
      node->apply_visitor(vis);
      return vis.result;
    }
  }
};

}  // namespace impl

template<typename T, typename U>
inline T* ast_node_cast(U* node) {
  return impl::cast_helper<T, U, impl::identity>::do_cast(node);
}

template<typename T, typename U>
inline const T* ast_node_cast(const U* node) {
  return impl::cast_helper<T, U, std::add_const_t>::do_cast(node);
}

}  // namespace mparse