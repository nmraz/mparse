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
  print_visitor(op_precedence parent_precedence, branch_side side, associativity parent_assoc,
    mparse::source_map* smap, std::size_t loc_offset = 0);

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

    return { begin + loc_offset, end + loc_offset };
  }

  op_precedence parent_precedence;
  bool assoc_paren;

  mparse::source_map* smap;
  std::size_t loc_offset;

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


print_visitor::print_visitor(op_precedence parent_precedence, branch_side side, associativity parent_assoc,
  mparse::source_map* smap, std::size_t loc_offset)
  : parent_precedence(parent_precedence)
  , assoc_paren(should_parenthesize_assoc(side, parent_assoc))
  , smap(smap)
  , loc_offset(loc_offset) {
}

void print_visitor::visit(const mparse::paren_node& node) {
  node.child()->apply_visitor(*this);

  if (smap) {
    smap->set_locs(&node, { smap->find_primary_loc(node.child()) });  // copy just primary location
  }
}

void print_visitor::visit(const mparse::abs_node& node) {
  parent_precedence = op_precedence::unknown;
  mparse::source_range outer_loc = record_loc([&] {
    result += "|";
    node.child()->apply_visitor(*this);
    result += "|";
  });

  if (smap) {
    smap->set_locs(&node, { outer_loc });
  }
}

void print_visitor::visit(const mparse::unary_op_node& node) {
  print_visitor child_vis(op_precedence::unary, branch_side::none, associativity::none, smap, result.size());
  node.child()->apply_visitor(child_vis);

  mparse::source_range op_loc;
  mparse::source_range expr_loc = record_loc([&] {
    auto_parenthesizer paren(*this, op_precedence::unary);
    
    op_loc = record_loc([&] {
      result += stringify_unary_op(node.type());
    });

    result += child_vis.result;
  });

  if (smap) {
    smap->set_locs(&node, { expr_loc, op_loc });
  }
}

void print_visitor::visit(const mparse::binary_op_node& node) {
  mparse::binary_op_type op = node.type();
  op_precedence prec = get_precedence(op);
  associativity assoc = get_associativity(op);

  std::string lhs_str, rhs_str;

  {
    print_visitor lhs_vis(prec, branch_side::left, assoc, smap, result.size());
    node.lhs()->apply_visitor(lhs_vis);
    lhs_str = std::move(lhs_vis.result);
  }

  {
    print_visitor rhs_vis(prec, branch_side::right, assoc, smap, result.size());
    node.rhs()->apply_visitor(rhs_vis);
    rhs_str = std::move(rhs_vis.result);
  }

  mparse::source_range op_loc;
  mparse::source_range expr_loc = record_loc([&] {
    auto_parenthesizer paren(*this, prec);

    result += lhs_str + " ";

    op_loc = record_loc([&] {
      result += stringify_binary_op(op);
    });

    result += " " + rhs_str;
  });

  if (smap) {
    smap->set_locs(&node, { expr_loc, op_loc });
  }
}

void print_visitor::visit(const mparse::literal_node& node) {
  std::ostringstream stream;
  stream << node.val();

  mparse::source_range tok_loc = record_loc([&] {
    result += stream.str();
  });

  if (smap) {
    smap->set_locs(&node, { tok_loc });
  }
}

void print_visitor::visit(const mparse::id_node& node) {
  mparse::source_range tok_loc = record_loc([&] {
    result += node.name();
  });

  if (smap) {
    smap->set_locs(&node, { tok_loc });
  }
}

}  // namespace


std::string pretty_print(const mparse::ast_node* node, mparse::source_map* smap) {
  print_visitor vis(op_precedence::unknown, branch_side::none, associativity::none, smap);
  node->apply_visitor(vis);
  return vis.result;
}

}  // namespace ast_ops