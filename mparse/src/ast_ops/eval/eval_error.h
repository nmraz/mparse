#pragma once

#include "mparse/ast.h"
#include "util/span.h"
#include <stdexcept>
#include <string_view>
#include <vector>

namespace ast_ops {

enum class eval_errc {
  unknown,
  div_by_zero,
  bad_pow,
  unbound_var,
  bad_func_call,
  out_of_range,
};

class eval_error : public std::runtime_error {
public:
  eval_error(std::string_view what, eval_errc code,
             const mparse::ast_node* node);

  eval_errc code() const { return code_; }
  const mparse::ast_node* node() const { return node_; }

private:
  eval_errc code_;
  const mparse::ast_node* node_;
};


class arity_error : public std::runtime_error {
public:
  arity_error(std::string_view what, std::size_t expected,
              std::size_t provided);

  std::size_t expected() const { return expected_; }
  std::size_t provided() const { return provided_; }

private:
  std::size_t expected_;
  std::size_t provided_;
};

class func_arg_error : public std::runtime_error {
public:
  func_arg_error(std::string_view what, std::vector<std::size_t> indices);

  util::span<const std::size_t> indices() const { return indices_; }

private:
  std::vector<std::size_t> indices_;
};

} // namespace ast_ops