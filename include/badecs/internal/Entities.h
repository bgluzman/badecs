#ifndef BADECS_INTERNAL_ENTITIES_H
#define BADECS_INTERNAL_ENTITIES_H

#include <badecs/Common.h>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace bad::internal {

/// \brief Storage of entities and the set of their components types.
class Entities {

public:
  /// \brief Reserves an entity ID without adding it to the set of entities.
  /// \return The reserved entity ID.
  EntityId reserve() noexcept { return entityCounter_++; }

  /// \brief Instantiates an entity that has been returned by `reserve()`.
  /// \param id The entity ID to instantiate.
  void instantiate(EntityId id) {
    if (entities_.count(id) == 0) {  // do not re-instantiate if exists
      entities_[id] = {};
    }
  }

  /// \brief Removes the given entity and returns its components.
  /// \param id The entity ID to remove.
  /// \return The components of the removed entity, or nullopt if the entity
  /// does not exist.
  std::optional<std::unordered_set<ComponentId>> remove(EntityId id) {
    if (auto it = entities_.find(id); it != entities_.end()) {
      auto components = it->second;
      entities_.erase(it);
      return components;
    }
    return std::nullopt;
  }

  /// \brief Returns true if the given entity exists.
  /// \param id The entity ID to check.
  /// \return True if the entity exists.
  bool has(EntityId id) const noexcept { return entities_.count(id) > 0; }

  /// \brief Adds a component to the given entity if it exists.
  /// \param entity The entity ID to which to add the component.
  /// \param component The component ID to add.
  /// \return True if the component was added, false if the entity does not
  /// exist.
  bool addComponent(EntityId entity, ComponentId component) {
    if (auto it = entities_.find(entity); it != entities_.end()) {
      it->second.insert(component);
      return true;
    }
    return false;
  }

  /// \brief Returns true if the given entity has the given component.
  /// \param entity The entity ID to check.
  /// \param component The component ID to check.
  /// \return True if the entity has the component.
  bool hasComponent(EntityId entity, ComponentId component) const {
    if (auto it = entities_.find(entity); it != entities_.end()) {
      return it->second.count(component) > 0;
    }
    return false;
  }

  /// \brief Removes a component from the given entity if it exists.
  /// \param entity The entity ID from which to remove the component.
  /// \param component The component ID to remove.
  void removeComponent(EntityId entity, ComponentId component) {
    if (auto it = entities_.find(entity); it != entities_.end()) {
      it->second.erase(component);
    }
  }

private:
  EntityId entityCounter_ = 1;
  std::unordered_map<EntityId, std::unordered_set<ComponentId>> entities_ = {};
};

}  // namespace bad::internal

#endif
