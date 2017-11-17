#pragma once

#include <type_traits>

namespace mparse {

class ast_node;
class unary_node;
class paren_node;
class unary_op_node;
class binary_op_node;
class literal_node;

struct ast_visitor {
  virtual void visit(ast_node&) {}
  virtual void visit(unary_node&) {}
  virtual void visit(paren_node&) {}
  virtual void visit(unary_op_node&) {}
  virtual void visit(binary_op_node&) {}
  virtual void visit(literal_node&) {}
};


template<typename To, typename From>
To* ast_node_cast(From* node) {
  static_assert(std::is_base_of_v<ast_node, To>, "Node casts are only supported to AST nodes");
  static_assert(std::is_base_of_v<ast_node, From>, "Node casts are only supported from AST nodes");

  if constexpr (std::is_base_of_v<To, From>) {  // upcast
    return static_cast<To*>(node);
  } else {
    static_assert(std::is_base_of_v<From, To>, "ast_node_cast does not support sibling casts");

    if (!node) {
      return nullptr;
    }

    struct cast_visitor : ast_visitor {
      void visit(To& obj) override {
        result = &obj;
      }

      To* result = nullptr;
    };

    cast_visitor vis;
    node->apply_visitor(vis);
    return vis.result;
  }
}

}  // namespace mparse