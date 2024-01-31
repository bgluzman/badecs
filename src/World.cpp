#include "World.h"

namespace bad {

EntityHandle World::spawnEntity() {
  EntityId newEntityId = kEntityIdCounter++;
  entities_.insert(newEntityId);
  return EntityHandle(newEntityId, componentStorage_.get());
}

std::optional<EntityHandle> World::getEntity(EntityId id) {
  return entities_.contains(id) ? std::optional<EntityHandle>(
                                      EntityHandle(id, componentStorage_.get()))
                                : std::nullopt;
}

}  // namespace bad