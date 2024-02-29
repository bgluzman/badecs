#pragma once

#include "Common.h"
#include "ComponentRegistry.h"
#include "EntityRegistry.h"

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
  [[nodiscard]] bool has(EntityId id) const noexcept;

  template <Component T, typename... Ts>
  void emplaceComponent(EntityId entity, Ts&&...args);
  template <Component T>
  void setComponent(EntityId entity, const T& value);
  template <Component T>
  [[nodiscard]] bool hasComponent(EntityId entity) const noexcept;
  template <Component T>
  [[nodiscard]] T *getComponent(EntityId entity);

  template <Component Arg>
  decltype(auto) entitiesWithComponent();
  decltype(auto) allEntities();

  template <Component... Args>
  void forEach(ForEachFunctor<Args...> auto&& callback);

private:
  std::unique_ptr<EntityRegistry> entities_ =
      std::make_unique<EntityRegistry>();
  std::unique_ptr<ComponentRegistry> components_ =
      std::make_unique<ComponentRegistry>();
};

inline EntityId World::create() { return entities_->add(); }

inline bool World::has(EntityId id) const noexcept {
  return entities_->has(id);
}

template <Component T, typename... Ts>
void World::emplaceComponent(EntityId entity, Ts&&...args) {
  components_->emplace<T>(entity, std::forward<Ts>(args)...);
}

template <Component T>
void World::setComponent(EntityId entity, const T& value) {
  components_->set(entity, value);
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

inline decltype(auto) World::allEntities() { return entities_->entities(); }

template <Component... Args>
void World::forEach(ForEachFunctor<Args...> auto&& callback) {
  for (EntityId id : components_->entitiesWithComponents<Args...>()) {
    if constexpr (ForEachSimple<decltype(callback), Args...>) {
      callback(*components_->get<Args>(id)...);
    } else if constexpr (ForEachWithEntityId<decltype(callback), Args...>) {
      callback(id, *components_->get<Args>(id)...);
    } else {
      static_assert(always_false_v<decltype(callback)>,
                    "Invalid callback signature");
    }
  }
}

}  // namespace bad