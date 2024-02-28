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
  bool hasComponent(EntityId entity) const noexcept;
  template <Component T>
  T *getComponent(EntityId entity);

  template <Component... Args>
  void query(QueryFunctor<Args...> auto&& callback);

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

template <Component... Args>
void World::query(QueryFunctor<Args...> auto&& callback) {
  for (EntityId id : components_->getQueryComponents<Args...>()) {
    // TODO (bgluzman): create dedicated concept for this?
    if constexpr (std::is_invocable_v<decltype(callback), EntityId, Args...>) {
      callback(id, *components_->get<Args>(id)...);
    } else {
      callback(*components_->get<Args>(id)...);
    }
  }
}

}  // namespace bad