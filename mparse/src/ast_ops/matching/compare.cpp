#include "compare.h"

#include "ast_ops/matching/util.h"
#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/id_node.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/paren_node.h"

namespace ast_ops::matching {
namespace {

template<typename To>
const To* cast_ignore_paren(const mparse::ast_node* node) {
  struct cast_visitor : mparse::const_ast_visitor {
    void visit(const mparse::paren_node& node) override {
      node.child()->apply_visitor(*this);
    }

    void visit(const To& node) override {
      result = &node;
    }

    const To* result = nullptr;
  };

  cast_visitor vis;
  node->apply_visitor(vis);
  return vis.result;
}

struct compare_visitor : mparse::const_ast_visitor {
  explicit compare_visitor(const mparse::ast_node* other, compare_cache& cache);

  void visit(const mparse::paren_node& node) override;
  void visit(const mparse::unary_op_node& node) override;
  void visit(const mparse::binary_op_node& node) override;
  void visit(const mparse::id_node& node) override;
  void visit(const mparse::literal_node& node) override;

  const mparse::ast_node* other;
  compare_cache& cache;

  bool result;
};

compare_visitor::compare_visitor(const mparse::ast_node* other, compare_cache& cache)
  : other(other)
  , cache(cache) {
}

void compare_visitor::visit(const mparse::paren_node& node) {
  node.child()->apply_visitor(*this);
}

void compare_visitor::visit(const mparse::unary_op_node& node) {
  if (auto other_unary = cast_ignore_paren<mparse::unary_op_node>(other)) {
    result = node.type() == other_unary->type()
      && compare_exprs(node.child(), other_unary->child(), cache);
    return;
  }
  result = false;
}

void compare_visitor::visit(const mparse::binary_op_node& node) {
  if (auto other_binary = cast_ignore_paren<mparse::binary_op_node>(other)) {
    if (node.type() != other_binary->type()) {
      result = false;
      return;
    }

    if (compare_exprs(node.lhs(), other_binary->lhs(), cache)
      && compare_exprs(node.rhs(), other_binary->rhs(), cache)) {
      result = true;
      return;
    } else if (is_commutative(node.type())) {
      result = compare_exprs(node.lhs(), other_binary->rhs(), cache)
        && compare_exprs(node.rhs(), other_binary->lhs(), cache);
      return;
    }
  }
  result = false;
}

void compare_visitor::visit(const mparse::id_node& node) {
  if (auto other_lit = cast_ignore_paren<mparse::id_node>(other)) {
    result = node.name() == other_lit->name();
    return;
  }
  result = false;
}

void compare_visitor::visit(const mparse::literal_node& node) {
  if (auto other_lit = cast_ignore_paren<mparse::literal_node>(other)) {
    result = node.val() == other_lit->val();
    return;
  }
  result = false;
}

}  // namespace


bool compare_exprs(const mparse::ast_node* first, const mparse::ast_node* second, compare_cache& cache) {
  if (first == second) {
    return true;
  }

  impl::compare_cache_key key = { first, second };
  if (auto it = cache.find(key); it != cache.end()) {
    return it->second;
  }

  compare_visitor vis(second, cache);
  first->apply_visitor(vis);
  return cache[key] = vis.result;
}

}  // namespace ast_ops::matching