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
  void query(QueryFunctor<Args...> auto&& callback);

private:
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
void World::query(QueryFunctor<Args...> auto&& callback) {
  for (EntityId id : components_->getQueryComponents<Args...>()) {
    // TODO (bgluzman): create dedicated concept for this?
    if constexpr (std::is_invocable_v<decltype(callback), EntityHandle,
                                      Args...>) {
      callback(*getEntity(id), components_->getUnchecked<Args>(id)...);
    } else {
      callback(components_->getUnchecked<Args>(id)...);
    }
  }
}

}  // namespace bad