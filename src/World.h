#pragma once

#include "Common.h"
#include "ComponentManager.h"
#include "EntityHandle.h"

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
  std::set<EntityId> getQueryComponents();

  // TODO (bgluzman): should not be static!
  static inline EntityId            kEntityIdCounter = 1;
  std::unordered_set<EntityId>      entities_ = {};
  std::unique_ptr<ComponentManager> componentStorage_ =
      std::make_unique<ComponentManager>();
};

// TODO (bgluzman): actually implement these properly...
template <Component... Args>
void World::query(std::invocable<Args...> auto&& callback) {
  for (EntityId id : getQueryComponents<Args...>()) {
    // TODO (bgluzman): change return types to make this better or provide
    //  a getUnchecked member function?
    callback(**componentStorage_->get<Args>(id)...);
  }
}

template <Component... Args>
void World::query(std::invocable<EntityHandle, Args...> auto&& callback) {
  // TODO (bgluzman): DRY w.r.t above?
  for (EntityId id : getQueryComponents<Args...>()) {
    // TODO (bgluzman): change return types to make this better or provide
    //  a getUnchecked member function?
    callback(*getEntity(id), **componentStorage_->get<Args>(id)...);
  }
}

template <Component Arg, Component... Args>
std::set<EntityId> World::getQueryComponents() {
  if constexpr (sizeof...(Args) == 0) {
    return componentStorage_->getColumn<Arg>().components | std::views::keys |
           std::ranges::to<std::set<EntityId>>();
  } else {
    // TODO (bgluzman): could we reuse the set from the recursive call?
    std::set<EntityId> result;
    std::ranges::set_intersection(
        // TODO (bgluzman): can probably get around making this intermediate
        //  set; just have to maintain sorted order...
        componentStorage_->getColumn<Arg>().components | std::views::keys |
            std::ranges::to<std::set<EntityId>>(),
        getQueryComponents<Args...>(), std::inserter(result, result.begin()));
    return result;
  }
}

}  // namespace bad