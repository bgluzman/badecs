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
  [[nodiscard]] bool contains(EntityId id) const noexcept;

  template <Component T, typename... Ts>
  void emplace(EntityId id, Ts&&...args);
  template <Component T>
  T *get(EntityId id);
  template <Component T>
  void set(EntityId id, const T& value);

  template <Component... Args>
  void query(QueryFunctor<Args...> auto&& callback);

private:
  std::unique_ptr<EntityRegistry> entities_ =
      std::make_unique<EntityRegistry>();
  std::unique_ptr<ComponentRegistry> components_ =
      std::make_unique<ComponentRegistry>();
};

inline EntityId World::create() { return entities_->add(); }

inline bool World::contains(EntityId id) const noexcept {
  return entities_->has(id);
}

template <Component T, typename... Ts>
void World::emplace(EntityId id, Ts&&...args) {
  components_->emplace<T, Ts...>(id, std::forward<Ts>(args)...);
}

template <Component T>
T *World::get(EntityId id) {
  return components_->get<T>(id);
}

template <Component T>
void World::set(EntityId id, const T& value) {
  components_->set(id, value);
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