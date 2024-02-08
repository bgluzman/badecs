#pragma once

#include "Common.h"

#include <optional>
#include <unordered_set>

namespace bad {

class EntityManager {
public:
  EntityId add();
  bool     has(EntityId id) noexcept;

private:
  EntityId                     entity_counter_ = 1;
  std::unordered_set<EntityId> entities_ = {};
};

inline EntityId EntityManager::add() {
  entities_.insert(entity_counter_);
  return entity_counter_++;
}

inline bool EntityManager::has(EntityId id) noexcept {
  return entities_.contains(id);
}

}  // namespace bad