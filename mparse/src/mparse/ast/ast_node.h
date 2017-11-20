#pragma once

#include "mparse/ast/ast_visitor.h"
#include "mparse/source_range.h"
#include <memory>
#include <type_traits>

namespace mparse {

class ast_node {
public:
  ast_node(const ast_node&) = delete;
  ast_node(ast_node&&) = delete;
  ast_node& operator=(const ast_node&) = delete;
  ast_node& operator=(ast_node&&) = delete;

  virtual ~ast_node() = 0;

  virtual void apply_visitor(ast_visitor& vis);
  virtual void apply_visitor(const_ast_visitor& vis) const;
};

using ast_node_ptr = std::unique_ptr<ast_node>;

template<typename Node, typename... Args>
std::unique_ptr<Node> make_ast_node(Args&&... args) {
  static_assert(std::is_base_of_v<ast_node, Node>, "make_ast_node can only be used for AST nodes");
  return std::make_unique<Node>(std::forward<Args>(args)...);
}


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