#include "pretty_print.h"

#include "op_strings.h"
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

class auto_parenthesizer {
public:
  auto_parenthesizer(std::string& expr, op_precedence parent_precedence, op_precedence precedence);
  ~auto_parenthesizer();

private:
  std::string& expr_;
  bool parenthesize_;
};

auto_parenthesizer::auto_parenthesizer(std::string& expr, op_precedence parent_precedence, op_precedence precedence)
  : expr_(expr)
  , parenthesize_(static_cast<int>(precedence) < static_cast<int>(parent_precedence)) {
  if (parenthesize_) {
    expr_ += "(";
  }
}

auto_parenthesizer::~auto_parenthesizer() {
  if (parenthesize_) {
    expr_ += ")";
  }
}


struct print_visitor : mparse::const_ast_visitor {
  print_visitor(mparse::source_map* smap, op_precedence parent_precedence, std::size_t loc_offset = 0);

  void visit(const mparse::paren_node& node) override;
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

  mparse::source_map* smap;
  op_precedence parent_precedence = op_precedence::unknown;
  std::size_t loc_offset;

  std::string result;
};

print_visitor::print_visitor(mparse::source_map* smap, op_precedence parent_precedence, std::size_t loc_offset)
  : smap(smap)
  , parent_precedence(parent_precedence)
  , loc_offset(loc_offset) {
}

void print_visitor::visit(const mparse::paren_node& node) {
  node.child()->apply_visitor(*this);

  if (smap) {
    smap->set_locs(&node, { smap->find_primary_loc(node.child()) });  // copy just primary location
  }
}

void print_visitor::visit(const mparse::unary_op_node& node) {
  print_visitor child_vis(smap, op_precedence::unary, result.size());
  node.child()->apply_visitor(child_vis);

  mparse::source_range op_loc;
  mparse::source_range expr_loc = record_loc([&] {
    auto_parenthesizer paren(result, parent_precedence, op_precedence::unary);
    
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

  print_visitor child_vis(smap, prec, result.size());

  node.lhs()->apply_visitor(child_vis);
  std::string lhs_str = std::move(child_vis.result);

  child_vis.result.clear();
  node.rhs()->apply_visitor(child_vis);
  std::string rhs_str = std::move(child_vis.result);

  mparse::source_range op_loc;
  mparse::source_range expr_loc = record_loc([&] {
    auto_parenthesizer paren(result, parent_precedence, prec);

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
  print_visitor vis(smap, op_precedence::unknown);
  node->apply_visitor(vis);
  return vis.result;
}

}  // namespace ast_ops