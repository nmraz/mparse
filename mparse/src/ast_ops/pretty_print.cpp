#include "pretty_print.h"

#include "op_strings.h"
#include "mparse/ast/abs_node.h"
#include "mparse/ast/ast_visitor.h"
#include "mparse/ast/id_node.h"
#include "mparse/ast/literal_node.h"
#include "mparse/ast/operator_nodes.h"
#include "mparse/ast/paren_node.h"
#include "util/auto_restore.h"
#include <sstream>

using namespace std::literals;

namespace ast_ops {
namespace {

// all operators, in order of increasing precedence
enum class op_precedence {
  unknown,
  add,
  mult,
  unary,
  pow
};

op_precedence get_precedence(mparse::binary_op_type op) {
  switch (op) {
  case mparse::binary_op_type::add:
  case mparse::binary_op_type::sub:
    return op_precedence::add;

  case mparse::binary_op_type::mult:
  case mparse::binary_op_type::div:
    return op_precedence::mult;

  case mparse::binary_op_type::pow:
    return op_precedence::pow;
  }

  return op_precedence::unknown;  // force parenthesization
}


// whether we are on the right or left-hand side of a subexpression
enum class branch_side {
  left,
  right,
  none  // unary nodes
};

// default associativity of unparenthesized operators - left, right, or fully associative
enum class associativity {
  left,
  right,
  both,
  none  // unary
};

associativity get_associativity(mparse::binary_op_type op) {
  switch (op) {
  case mparse::binary_op_type::sub:
  case mparse::binary_op_type::div:
    return associativity::left;
  case mparse::binary_op_type::pow:
    return associativity::right;
  case mparse::binary_op_type::add:
  case mparse::binary_op_type::mult:
    return associativity::both;
  }
  return associativity::none;
}

bool should_parenthesize_assoc(branch_side side, associativity parent_assoc) {
  if (parent_assoc == associativity::both) {
    return false;
  }
  return parent_assoc == associativity::none
    || (side == branch_side::left && parent_assoc == associativity::right)
    || (side == branch_side::right && parent_assoc == associativity::left);  // associativity doesn't match side
}

bool should_parenthesize(op_precedence parent_precedence, op_precedence precedence, bool assoc_paren) {
  if (precedence == parent_precedence) {  // tie - determine based on associativity
    return assoc_paren;
  }
  return static_cast<int>(precedence) < static_cast<int>(parent_precedence);
}


struct print_visitor : mparse::const_ast_visitor {
  print_visitor(mparse::source_map* smap) : smap(smap) {}

  void visit(const mparse::paren_node& node) override;
  void visit(const mparse::abs_node& node) override;
  void visit(const mparse::unary_op_node& node) override;
  void visit(const mparse::binary_op_node& node) override;
  void visit(const mparse::literal_node& node) override;
  void visit(const mparse::id_node& node) override;

  template<typename F>
  mparse::source_range record_loc(F&& f) const {
    std::size_t begin = result.size();
    std::forward<F>(f)();
    std::size_t end = result.size();

    return { begin, end };
  }

  op_precedence parent_precedence;
  bool assoc_paren;

  mparse::source_map* smap;
  std::string result;
};


class auto_parenthesizer {
public:
  auto_parenthesizer(print_visitor& vis, op_precedence precedence);
  ~auto_parenthesizer();

private:
  std::string& expr_;
  bool parenthesize_;
};

auto_parenthesizer::auto_parenthesizer(print_visitor& vis, op_precedence precedence)
  : expr_(vis.result)
  , parenthesize_(should_parenthesize(vis.parent_precedence, precedence, vis.assoc_paren)) {
  if (parenthesize_) {
    expr_ += "(";
  }
}

auto_parenthesizer::~auto_parenthesizer() {
  if (parenthesize_) {
    expr_ += ")";
  }
}


class child_visitor_scope {
public:
  child_visitor_scope(print_visitor& vis, op_precedence parent_precedence,
    associativity parent_assoc, branch_side side);
  ~child_visitor_scope();

private:
  print_visitor& vis_;
  op_precedence old_precedence_;
  bool old_assoc_paren_;
};

child_visitor_scope::child_visitor_scope(print_visitor& vis, op_precedence parent_precedence,
  associativity parent_assoc, branch_side side)
  : vis_(vis)
  , old_precedence_(vis.parent_precedence)
  , old_assoc_paren_(vis.assoc_paren) {
  vis.parent_precedence = parent_precedence;
  vis.assoc_paren = should_parenthesize_assoc(side, parent_assoc);
}

child_visitor_scope::~child_visitor_scope() {
  vis_.parent_precedence = old_precedence_;
  vis_.assoc_paren = old_assoc_paren_;
}

}  // namespace


std::string pretty_print(const mparse::ast_node* node, mparse::source_map* smap) {
}

}  // namespace ast_ops