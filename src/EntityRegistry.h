#pragma once

#include "Common.h"

#include <optional>
#include <unordered_set>

namespace bad {

class EntityRegistry {
public:
  EntityId           add();
  [[nodiscard]] bool has(EntityId id) const noexcept;

private:
  EntityId                     entity_counter_ = 1;
  std::unordered_set<EntityId> entities_ = {};
};

inline EntityId EntityRegistry::add() {
  entities_.insert(entity_counter_);
  return entity_counter_++;
}

inline bool EntityRegistry::has(EntityId id) const noexcept {
  return entities_.contains(id);
}

}  // namespace bad