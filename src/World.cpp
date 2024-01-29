#include "World.h"

namespace bad {

EntityHandle World::spawnEntity() {
  return EntityHandle(kEntityIdCounter++, componentStorage_.get());
}

std::optional<EntityHandle> World::getEntity(EntityId id) {
  return entities_.contains(id) ? std::optional<EntityHandle>(
                                      EntityHandle(id, componentStorage_.get()))
                                : std::nullopt;
}

}  // namespace bad