#pragma once

#include "mparse/ast/root_node.h"
#include <memory>
#include <vector>

namespace mparse {

class abstract_syntax_tree {
public:
  abstract_syntax_tree();

  root_node* root();
  const root_node* root() const;

  template<typename Node, typename... Args>
  Node* make_node(Args&&... args);

private:
  std::vector<std::unique_ptr<ast_node>> nodes_;
};

template<typename Node, typename... Args>
Node* abstract_syntax_tree::make_node(Args&&... args) {
  static_assert(std::is_base_of_v<ast_node, Node>, "Attempting to add non-node to AST");
  // std::make_unique doesn't have access to the protected ctor
  std::unique_ptr<Node> node(new Node(std::forward<Args>(args)...));

  Node* raw_node = node.get();
  nodes_.push_back(std::move(node));
  return raw_node;
}

}  // namespace mparse