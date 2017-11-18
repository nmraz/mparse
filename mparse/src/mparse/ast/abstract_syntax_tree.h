#pragma once

#include "mparse/ast/ast_node.h"
#include <memory>
#include <vector>

namespace mparse {

class abstract_syntax_tree {
public:
  abstract_syntax_tree() = default;
  abstract_syntax_tree(abstract_syntax_tree&& rhs);

  abstract_syntax_tree& operator=(abstract_syntax_tree&& rhs);

  void set_root(ast_node* root);

  ast_node* root() { return root_; }
  const ast_node* root() const { return root_; }

  template<typename Node, typename... Args>
  Node* make_node(Args&&... args);

private:
  std::vector<std::unique_ptr<ast_node>> nodes_;
  ast_node* root_ = nullptr;
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