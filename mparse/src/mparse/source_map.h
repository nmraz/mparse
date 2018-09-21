#pragma once

#include "mparse/source_range.h"
#include "util/span.h"
#include <unordered_map>
#include <vector>

namespace mparse {

class ast_node;

class source_map {
public:
  void set_locs(const ast_node* node, std::vector<source_range> locs);
  void clear();

  util::span<const source_range> find_locs(const ast_node* node) const;
  source_range find_primary_loc(const ast_node* node) const {
    return find_locs(node)[0];
  }

private:
  std::unordered_map<const ast_node*, std::vector<source_range>> loc_map_;
};

} // namespace mparse