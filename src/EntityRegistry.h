#pragma once

#include "Common.h"

#include <map>
#include <optional>
#include <ranges>
#include <unordered_set>

namespace bad {

class EntityRegistry {
public:
  EntityId add();
  EntityId addDeferred();
  void     instantiateDeferred(EntityId id);
  std::optional<std::unordered_set<ComponentId>> remove(EntityId id);
  [[nodiscard]] bool has(EntityId id) const noexcept;

  void addComponent(EntityId entity, ComponentId component);
  void removeComponent(EntityId entity, ComponentId component);

  [[nodiscard]] decltype(auto) entities() const;

private:
  EntityId                                            entity_counter_ = 1;
  std::map<EntityId, std::unordered_set<ComponentId>> entities_ = {};
};

inline EntityId EntityRegistry::add() {
  entities_.emplace(entity_counter_, std::unordered_set<ComponentId>{});
  return entity_counter_++;
}

inline EntityId EntityRegistry::addDeferred() { return entity_counter_++; }

inline void EntityRegistry::instantiateDeferred(EntityId id) {
  entities_.emplace(id, std::unordered_set<ComponentId>{});
}

inline std::optional<std::unordered_set<ComponentId>>
EntityRegistry::remove(EntityId id) {
  if (!has(id)) {
    return std::nullopt;
  }
  return std::move(entities_.extract(id).mapped());
}

inline bool EntityRegistry::has(EntityId id) const noexcept {
  return entities_.contains(id);
}

inline void EntityRegistry::addComponent(EntityId    entity,
                                         ComponentId component) {
  if (has(entity))
    entities_[entity].insert(component);
}

inline void EntityRegistry::removeComponent(EntityId    entity,
                                            ComponentId component) {
  if (has(entity))
    entities_[entity].erase(component);
}

inline decltype(auto) EntityRegistry::entities() const {
  return entities_ | std::views::keys;
}

}  // namespace bad