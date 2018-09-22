#include "error_handling.h"

#include "diagnostics.h"
#include "mparse/ast/func_node.h"
#include "mparse/ast/operator_nodes.h"
#include <iostream>
#include <sstream>

using namespace std::literals;

namespace {

void print_math_error(std::string_view msg) {
  print_error("Math", msg);
}

void print_syntax_error(std::string_view msg) {
  print_error("Syntax", msg);
}


void handle_bad_func_call(const ast_ops::eval_error& err,
                          const mparse::source_map& smap,
                          std::string_view input) {
  auto* node = static_cast<const mparse::func_node*>(err.node());

  std::ostringstream msg;
  std::vector<mparse::source_range> locs = {
      smap.find_locs(node)[1], // function name
  };

  msg << err.what();

  try {
    std::rethrow_if_nested(err);
  } catch (const ast_ops::arity_error& arity_err) {
    msg << ": " << arity_err.what();

    auto expected = arity_err.expected();
    auto provided = arity_err.provided();

    if (expected < provided) {
      for (const auto& arg :
           util::span{node->args()}.last(provided - expected)) {
        locs.push_back(smap.find_primary_loc(arg.get()));
      }
    }
  } catch (const ast_ops::func_arg_error& arg_err) {
    msg << ": " << arg_err.what();

    for (auto index : arg_err.indices()) {
      locs.push_back(smap.find_primary_loc(node->args()[index].get()));
    }
  } catch (const std::exception& inner) {
    msg << ": " << inner.what();
  } catch (...) {
  }

  print_math_error(msg.str());
  print_locs(input, locs);
}

} // namespace


void handle_syntax_error(const mparse::syntax_error& err,
                         std::string_view input) {
  print_syntax_error(err.what());
  print_locs(input, err.where());
  if (!err.fixit_hint().empty()) {
    print_fixit(err.fixit_hint(), err.fixit_col());
  }
}

void handle_math_error(const ast_ops::eval_error& err,
                       const mparse::source_map& smap, std::string_view input) {
  auto* node = static_cast<const mparse::binary_op_node*>(err.node());

  switch (err.code()) {
  case ast_ops::eval_errc::div_by_zero:
    print_math_error(err.what());
    print_loc(input, smap.find_primary_loc(node->rhs()));
    break;
  case ast_ops::eval_errc::bad_pow:
    print_math_error(err.what());
    print_locs(input, {smap.find_primary_loc(node->lhs()),
                       smap.find_primary_loc(node->rhs())});
    break;
  case ast_ops::eval_errc::unbound_var:
    print_math_error(err.what());
    print_loc(input, smap.find_primary_loc(err.node()));
    break;
  case ast_ops::eval_errc::bad_func_call:
    handle_bad_func_call(err, smap, input);
    break;
  case ast_ops::eval_errc::out_of_range:
    print_math_error(err.what());
    print_locs(input, {smap.find_primary_loc(err.node())});
    break;
  default:
    print_math_error(err.what());
    break;
  }
}