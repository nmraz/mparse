#include "source_map.h"

namespace mparse {

void source_map::set_locs(const ast_node* node,
                          std::vector<source_range> locs) {
  loc_map_[node] = std::move(locs);
}

util::span<const source_range> source_map::find_locs(
    const ast_node* node) const {
  auto it = loc_map_.find(node);

  if (it == loc_map_.end()) {
    return {};
  }
  return it->second;
}


void source_map::clear() {
  loc_map_.clear();
}

} // namespace mparse