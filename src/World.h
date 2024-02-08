#pragma once

#include "Common.h"
#include "ComponentRegistry.h"
#include "EntityHandle.h"
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
  EntityHandle                spawnEntity();
  std::optional<EntityHandle> getEntity(EntityId id);

  template <Component... Args>
  void query(std::invocable<Args...> auto&& callback);
  template <Component... Args>
  void query(std::invocable<EntityHandle, Args...> auto&& callback);

private:
  template <Component Arg, Component... Args>
  std::vector<EntityId> getQueryComponents();

  std::unique_ptr<EntityRegistry> entities_ =
      std::make_unique<EntityRegistry>();
  std::unique_ptr<ComponentRegistry> components_ =
      std::make_unique<ComponentRegistry>();
};

inline EntityHandle World::spawnEntity() {
  return EntityHandle(entities_->add(), components_.get());
}

inline std::optional<EntityHandle> World::getEntity(EntityId id) {
  return entities_->has(id)
             ? std::optional<EntityHandle>(EntityHandle(id, components_.get()))
             : std::nullopt;
}

template <Component... Args>
void World::query(std::invocable<Args...> auto&& callback) {
  for (EntityId id : getQueryComponents<Args...>()) {
    callback(components_->getUnchecked<Args>(id)...);
  }
}

template <Component... Args>
void World::query(std::invocable<EntityHandle, Args...> auto&& callback) {
  for (EntityId id : getQueryComponents<Args...>()) {
    callback(*getEntity(id), components_->getUnchecked<Args>(id)...);
  }
}

template <Component Arg, Component... Args>
std::vector<EntityId> World::getQueryComponents() {
  if constexpr (sizeof...(Args) == 0) {
    return components_->getColumn<Arg>().getEntityIds();
  } else {
    auto argQueryComponents = components_->getColumn<Arg>().getEntityIds();
    auto argsQueryComponents = getQueryComponents<Args...>();

    std::vector<EntityId> result;
    std::ranges::set_intersection(argQueryComponents, argsQueryComponents,
                                  std::back_inserter(result));
    return result;
  }
}

}  // namespace bad