#pragma once

#include "mparse/ast/ast_visitor.h"
#include "mparse/source_range.h"
#include <memory>
#include <type_traits>

namespace mparse {

class ast_node {
public:
  constexpr ast_node() = default;

  ast_node(const ast_node&) = delete;
  ast_node(ast_node&&) = delete;
  ast_node& operator=(const ast_node&) = delete;
  ast_node& operator=(ast_node&&) = delete;

  virtual ~ast_node() = 0;

  virtual void apply_visitor(ast_visitor& vis);
  virtual void apply_visitor(const_ast_visitor& vis) const;

  virtual bool has_id(const void* id) const;
};

namespace impl {

template <typename T>
struct get_node_ptr {
  static_assert(std::is_base_of_v<ast_node, T>,
                "node_ptr can only be used for AST nodes");
  using type = std::shared_ptr<T>;
};

} // namespace impl

template <typename T>
using node_ptr = typename impl::get_node_ptr<T>::type;
using ast_node_ptr = node_ptr<ast_node>;

template <typename Node, typename... Args>
node_ptr<Node> make_ast_node(Args&&... args) {
  static_assert(std::is_base_of_v<ast_node, Node>,
                "make_ast_node can only be used for AST nodes");
  return std::make_shared<Node>(std::forward<Args>(args)...);
}

} // namespace mparse