#include "pretty_print.h"

#include "mparse/ast.h"
#include "op_strings.h"
#include "util/auto_restore.h"
#include <iomanip>
#include <limits>
#include <sstream>

using namespace std::literals;

namespace ast_ops {
namespace {

// all operators, in order of increasing precedence
enum class op_precedence { unknown, add, mult, unary, pow };

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

  return op_precedence::unknown; // force parenthesization
}


// whether we are on the right or left-hand side of a subexpression
enum class branch_side {
  left,
  right,
  none // unary nodes
};

// default associativity of unparenthesized operators - left, right, or fully
// associative
enum class associativity {
  left,
  right,
  both,
  none // unary
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
  return parent_assoc == associativity::none ||
         (side == branch_side::left && parent_assoc == associativity::right) ||
         (side == branch_side::right &&
          parent_assoc ==
              associativity::left); // associativity doesn't match side
}

bool should_parenthesize(op_precedence parent_precedence,
                         op_precedence precedence, bool assoc_paren) {
  if (precedence ==
      parent_precedence) { // tie - determine based on associativity
    return assoc_paren;
  }
  return static_cast<int>(precedence) < static_cast<int>(parent_precedence);
}


struct print_visitor : mparse::const_ast_visitor<print_visitor> {
  explicit print_visitor(mparse::source_map* smap) : smap(smap) {}

  void operator()(const mparse::paren_node& node);
  void operator()(const mparse::abs_node& node);
  void operator()(const mparse::unary_op_node& node);
  void operator()(const mparse::binary_op_node& node);
  void operator()(const mparse::func_node& node);
  void operator()(const mparse::literal_node& node);
  void operator()(const mparse::id_node& node);

  void set_locs(const mparse::ast_node& node,
                std::vector<mparse::source_range> locs);

  template <typename F>
  mparse::source_range record_loc(F&& f) const {
    std::size_t begin = result.size();
    std::forward<F>(f)();
    std::size_t end = result.size();

    return {begin, end};
  }

  op_precedence parent_precedence = op_precedence::unknown;
  bool assoc_paren =
      should_parenthesize_assoc(branch_side::none, associativity::none);

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

auto_parenthesizer::auto_parenthesizer(print_visitor& vis,
                                       op_precedence precedence)
    : expr_(vis.result),
      parenthesize_(should_parenthesize(vis.parent_precedence, precedence,
                                        vis.assoc_paren)) {
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

private:
  util::auto_restore<op_precedence> restore_precedence_;
  util::auto_restore<bool> restore_assoc_paren_;
};

child_visitor_scope::child_visitor_scope(print_visitor& vis,
                                         op_precedence parent_precedence,
                                         associativity parent_assoc,
                                         branch_side side)
    : restore_precedence_(vis.parent_precedence, parent_precedence),
      restore_assoc_paren_(vis.assoc_paren,
                           should_parenthesize_assoc(side, parent_assoc)) {}


void print_visitor::operator()(const mparse::paren_node& node) {
  mparse::source_range loc = record_loc([&] {
    result += "(";
    {
      child_visitor_scope scope(*this, op_precedence::unknown,
                                associativity::none, branch_side::none);
      mparse::apply_visitor(*this, *node.child());
    }
    result += ")";
  });

  set_locs(node, {loc});
}

void print_visitor::operator()(const mparse::abs_node& node) {
  mparse::source_range loc = record_loc([&] {
    result += "|";
    {
      child_visitor_scope scope(*this, op_precedence::unknown,
                                associativity::none, branch_side::none);
      mparse::apply_visitor(*this, *node.child());
    }
    result += "|";
  });

  set_locs(node, {loc});
}

void print_visitor::operator()(const mparse::unary_op_node& node) {
  mparse::source_range op_loc;
  mparse::source_range expr_loc = record_loc([&] {
    auto_parenthesizer paren(*this, op_precedence::unary);

    op_loc = record_loc([&] { result += stringify_unary_op(node.type()); });

    child_visitor_scope scope(*this, op_precedence::unary, associativity::none,
                              branch_side::none);
    mparse::apply_visitor(*this, *node.child());
  });

  set_locs(node, {expr_loc, op_loc});
}

void print_visitor::operator()(const mparse::binary_op_node& node) {
  op_precedence prec = get_precedence(node.type());
  associativity assoc = get_associativity(node.type());

  mparse::source_range op_loc;
  mparse::source_range expr_loc = record_loc([&] {
    auto_parenthesizer paren(*this, prec);

    {
      child_visitor_scope scope(*this, prec, assoc, branch_side::left);
      mparse::apply_visitor(*this, *node.lhs());
    }

    result += " ";
    op_loc = record_loc([&] { result += stringify_binary_op(node.type()); });
    result += " ";

    {
      child_visitor_scope scope(*this, prec, assoc, branch_side::right);
      mparse::apply_visitor(*this, *node.rhs());
    }
  });

  set_locs(node, {expr_loc, op_loc});
}

void print_visitor::operator()(const mparse::func_node& node) {
  mparse::source_range name_loc, open_loc;
  mparse::source_range expr_loc = record_loc([&] {
    name_loc = record_loc([&] { result += node.name(); });
    open_loc = record_loc([&] { result += "("; });

    {
      child_visitor_scope scope(*this, op_precedence::unknown,
                                associativity::none, branch_side::none);
      for (const auto& arg : node.args()) {
        if (arg != node.args().front()) {
          result += ", ";
        }
        mparse::apply_visitor(*this, *arg);
      }
    }

    result += ")";
  });

  set_locs(node, {expr_loc, name_loc, open_loc});
}

void print_visitor::operator()(const mparse::literal_node& node) {
  std::ostringstream stream;
  stream << std::setprecision(std::numeric_limits<double>::digits10 + 1)
         << node.val();

  mparse::source_range loc = record_loc([&] { result += stream.str(); });

  set_locs(node, {loc});
}

void print_visitor::operator()(const mparse::id_node& node) {
  mparse::source_range loc = record_loc([&] { result += node.name(); });

  set_locs(node, {loc});
}


void print_visitor::set_locs(const mparse::ast_node& node,
                             std::vector<mparse::source_range> locs) {
  if (smap) {
    smap->set_locs(&node, std::move(locs));
  }
}

} // namespace


std::string pretty_print(const mparse::ast_node& node,
                         mparse::source_map* smap) {
  print_visitor vis(smap);
  mparse::apply_visitor(vis, node);
  return vis.result;
}

} // namespace ast_ops