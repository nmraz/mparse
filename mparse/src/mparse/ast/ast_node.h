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

  virtual ~ast_node() = default;

  virtual void apply_visitor(ast_visitor& vis);
  virtual void apply_visitor(const_ast_visitor& vis) const;

protected:
  ast_node() = default;

private:
  friend class abstract_syntax_tree;
};

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