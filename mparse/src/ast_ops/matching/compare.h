#pragma once

#include "ast_ops/matching/util.h"
#include "mparse/ast.h"
#include <type_traits>

namespace ast_ops::matching {
namespace impl {

template <typename Comp>
struct compare_visitor : mparse::const_ast_visitor<compare_visitor<Comp>> {
  compare_visitor(const mparse::ast_node* other, Comp& comp);

  void operator()(const mparse::paren_node& node);
  void operator()(const mparse::abs_node& node);
  void operator()(const mparse::unary_op_node& node);
  void operator()(const mparse::binary_op_node& node);
  void operator()(const mparse::func_node& node);
  void operator()(const mparse::id_node& node);
  void operator()(const mparse::literal_node& node);

  const mparse::ast_node* other;
  Comp& comp;

  bool result = false;
};

template <typename Comp>
compare_visitor<Comp>::compare_visitor(const mparse::ast_node* other,
                                       Comp& comp)
    : other(other), comp(comp) {}

template <typename Comp>
void compare_visitor<Comp>::operator()(const mparse::paren_node& node) {
  if (auto* other_paren =
          mparse::ast_node_cast<const mparse::paren_node>(other)) {
    result = comp.compare_paren(node, *other_paren);
    return;
  }
  result = false;
}

template <typename Comp>
void compare_visitor<Comp>::operator()(const mparse::abs_node& node) {
  if (auto* other_abs = mparse::ast_node_cast<const mparse::abs_node>(other)) {
    result = comp.compare_abs(node, *other_abs);
    return;
  }
  result = false;
}

template <typename Comp>
void compare_visitor<Comp>::operator()(const mparse::unary_op_node& node) {
  if (auto* other_unary =
          mparse::ast_node_cast<const mparse::unary_op_node>(other)) {
    result = comp.compare_unary(node, *other_unary);
    return;
  }
  result = false;
}

template <typename Comp>
void compare_visitor<Comp>::operator()(const mparse::binary_op_node& node) {
  if (auto* other_binary =
          mparse::ast_node_cast<const mparse::binary_op_node>(other)) {
    result = comp.compare_binary(node, *other_binary);
    return;
  }
  result = false;
}

template <typename Comp>
void compare_visitor<Comp>::operator()(const mparse::func_node& node) {
  if (auto* other_func =
          mparse::ast_node_cast<const mparse::func_node>(other)) {
    result = comp.compare_func(node, *other_func);
    return;
  }
  result = false;
}

template <typename Comp>
void compare_visitor<Comp>::operator()(const mparse::id_node& node) {
  if (auto* other_id = mparse::ast_node_cast<const mparse::id_node>(other)) {
    result = comp.compare_id(node, *other_id);
    return;
  }
  result = false;
}

template <typename Comp>
void compare_visitor<Comp>::operator()(const mparse::literal_node& node) {
  if (auto* other_lit =
          mparse::ast_node_cast<const mparse::literal_node>(other)) {
    result = comp.compare_literal(node, *other_lit);
    return;
  }
  result = false;
}

} // namespace impl


template <typename Der>
struct default_expr_comparer_base {
  bool compare_paren(const mparse::paren_node& first,
                     const mparse::paren_node& second) const {
    return compare_exprs(*first.child(), *second.child(),
                         static_cast<const Der&>(*this));
  }

  bool compare_abs(const mparse::abs_node& first,
                   const mparse::abs_node& second) const {
    return compare_exprs(*first.child(), *second.child(),
                         static_cast<const Der&>(*this));
  }

  bool compare_unary(const mparse::unary_op_node& first,
                     const mparse::unary_op_node& second) const {
    return first.type() == second.type() &&
           compare_exprs(*first.child(), *second.child(),
                         static_cast<const Der&>(*this));
  }

  bool compare_binary(const mparse::binary_op_node& first,
                      const mparse::binary_op_node& second) const {
    if (first.type() != second.type()) {
      return false;
    }

    if (compare_exprs(*first.lhs(), *second.lhs(),
                      static_cast<const Der&>(*this)) &&
        compare_exprs(*first.rhs(), *second.rhs(),
                      static_cast<const Der&>(*this))) {
      return true;
    } else if (is_commutative(first.type()) &&
               compare_exprs(*first.lhs(), *second.rhs(),
                             static_cast<const Der&>(*this)) &&
               compare_exprs(*first.rhs(), *second.lhs(),
                             static_cast<const Der&>(*this))) {
      return true;
    }

    return false;
  }

  bool compare_func(const mparse::func_node& first,
                    const mparse::func_node& second) const {
    const auto& first_args = first.args();
    const auto& second_args = second.args();

    return first.name() == second.name() &&
           std::equal(first_args.begin(), first_args.end(), second_args.begin(),
                      second_args.end(),
                      [&](const auto& arg1, const auto& arg2) {
                        return compare_exprs(*arg1, *arg2,
                                             static_cast<const Der&>(*this));
                      });
  }

  bool compare_id(const mparse::id_node& first,
                  const mparse::id_node& second) const {
    return first.name() == second.name();
  }

  bool compare_literal(const mparse::literal_node& first,
                       const mparse::literal_node& second) const {
    return first.val() == second.val();
  }
};

struct default_expr_comparer
    : default_expr_comparer_base<default_expr_comparer> {};


template <typename Comp>
bool compare_exprs(const mparse::ast_node& first,
                   const mparse::ast_node& second, Comp&& comp) {
  impl::compare_visitor<std::remove_reference_t<Comp>> vis(&second, comp);
  mparse::apply_visitor(vis, first);
  return vis.result;
}

template <typename Comp = default_expr_comparer>
bool compare_exprs(const mparse::ast_node& first,
                   const mparse::ast_node& second) {
  return compare_exprs(first, second, Comp{});
}

} // namespace ast_ops::matching