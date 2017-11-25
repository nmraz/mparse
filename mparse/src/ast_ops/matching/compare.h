#pragma once

#include "mparse/ast/ast_node.h"
#include <unordered_map>

namespace ast_ops::matching {
namespace impl {

struct compare_cache_key {
  const mparse::ast_node* a;
  const mparse::ast_node* b;
};

constexpr bool operator==(const compare_cache_key& lhs, const compare_cache_key& rhs) {
  // unordered comparison
  return (lhs.a == rhs.a && lhs.b == rhs.b) || (lhs.a == rhs.b && lhs.b == rhs.a);
}

struct compare_key_hash {
  std::size_t operator()(const compare_cache_key& key) const {
    std::hash<const mparse::ast_node*> hasher;

    // Xor is okay (and even necessary) here because we want commutativity (the pairs are unordered).
    // In addition, the case of equal pointers is handeled before the cache is reached.
    return hasher(key.a) ^ hasher(key.b);
  }
};

}  // namespace impl

using compare_cache = std::unordered_map<impl::compare_cache_key, bool, impl::compare_key_hash>;

bool compare_exprs(const mparse::ast_node* first, const mparse::ast_node* second, compare_cache& cache);

}  // namespace ast_ops::matching