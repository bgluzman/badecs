#pragma once

#include "Common.h"
#include "ComponentRegistry.h"

#include <any>
#include <cstdint>
#include <gsl/gsl>
#include <iostream>
#include <ranges>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bad {

class World {
public:
  EntityId           create();
  bool               destroy(EntityId id);
  [[nodiscard]] bool has(EntityId id) const noexcept;

  template <Component T, typename... Ts>
  void emplaceComponent(EntityId entity, Ts&&...args);
  template <Component T>
  void setComponent(EntityId entity, const T& value);
  template <Component T>
  bool removeComponent(EntityId entity);
  template <Component T>
  [[nodiscard]] bool hasComponent(EntityId entity) const noexcept;
  template <Component T>
  [[nodiscard]] T *getComponent(EntityId entity);

  template <Component Arg>
  decltype(auto) entitiesWithComponent();
  decltype(auto) allEntities();

private:
  bool removeComponent(EntityId entity, ComponentId component);

  EntityId                                            entity_counter_ = 1;
  std::map<EntityId, std::unordered_set<ComponentId>> entities_ = {};

  std::unique_ptr<ComponentRegistry> components_ =
      std::make_unique<ComponentRegistry>();
};

inline EntityId World::create() {
  entities_.emplace(entity_counter_, std::unordered_set<ComponentId>{});
  return entity_counter_++;
}

inline bool World::destroy(EntityId entity) {
  if (auto it = entities_.find(entity); it != entities_.end()) {
    for (ComponentId component : entities_.extract(it).mapped()) {
      components_->remove(entity, component);
    }
    return true;
  }
  return false;
}

inline bool World::has(EntityId id) const noexcept {
  return entities_.contains(id);
}

template <Component T, typename... Ts>
void World::emplaceComponent(EntityId entity, Ts&&...args) {
  entities_[entity].emplace(components_->getComponentId<T>());
  components_->emplace<T>(entity, std::forward<Ts>(args)...);
}

template <Component T>
void World::setComponent(EntityId entity, const T& value) {
  entities_[entity].emplace(components_->getComponentId<T>());
  components_->set(entity, value);
}

template <Component T>
bool World::removeComponent(EntityId entity) {
  return removeComponent(entity, components_->getComponentId<T>());
}

inline bool World::removeComponent(EntityId entity, ComponentId component) {
  if (auto it = entities_.find(entity); it != entities_.end()) {
    if (it->second.erase(component) > 0) {
      return components_->remove(entity, component);
    }
  }
  return false;
}

template <Component T>
bool World::hasComponent(EntityId entity) const noexcept {
  return components_->has<T>(entity);
}

template <Component T>
T *World::getComponent(EntityId entity) {
  return components_->get<T>(entity);
}

template <Component Arg>
decltype(auto) World::entitiesWithComponent() {
  return components_->entitiesWithComponent<Arg>();
}

inline decltype(auto) World::allEntities() {
  return entities_ | std::views::keys;
}

}  // namespace bad