#pragma once

#include "Common.h"

#include <optional>
#include <ranges>
#include <set>

namespace bad {

class EntityRegistry {
public:
  EntityId           add();
  [[nodiscard]] bool has(EntityId id) const noexcept;

  [[nodiscard]] decltype(auto) entities() const;

private:
  EntityId           entity_counter_ = 1;
  std::set<EntityId> entities_ = {};
};

inline EntityId EntityRegistry::add() {
  entities_.insert(entity_counter_);
  return entity_counter_++;
}

inline bool EntityRegistry::has(EntityId id) const noexcept {
  return entities_.contains(id);
}

inline decltype(auto) EntityRegistry::entities() const {
  return std::ranges::ref_view(entities_);
}

}  // namespace bad