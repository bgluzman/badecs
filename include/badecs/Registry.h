#ifndef BADECS_REGISTRY_H
#define BADECS_REGISTRY_H

#include <badecs/Common.h>
#include <badecs/Filter.h>
#include <badecs/View.h>
#include <badecs/internal/Components.h>
#include <badecs/internal/Entities.h>

namespace bad {

/// \brief The top-level registry of entity-component relationships.
///
/// Maintains entities and their constituent components. Allows creation,
/// modification, and deletion of entities/components along with querying
/// for entities with specific components.
class Registry {

public:
  /// Creates a new entity.
  /// \return The entity-id of the newly created entity.
  EntityId createEntity() {
    EntityId id = reserveEntity();
    instantiateEntity(id);
    return id;
  }

  /// Reserves an entity-id without instantiating it.
  /// \return The reserved entity-id.
  EntityId reserveEntity() { return entities_.reserve(); }

  /// Instantiates an entity with the given entity-id.
  /// \param id The entity-id of the entity to instantiate.
  void instantiateEntity(EntityId id) { entities_.instantiate(id); }

  /// Destroys an entity with the given entity-id.
  /// \param id The entity-id of the entity to destroy.
  bool destroyEntity(EntityId id) {
    if (auto components = entities_.remove(id); components.has_value()) {
      // XXX: Cannot call removeComponent since the entity is already removed.
      components_.remove(id, *components);
      return true;
    }
    return false;
  }

  /// Checks if an entity with the given entity-id exists.
  /// \param id The entity-id to check.
  /// \return True if the entity exists, false otherwise.
  bool hasEntity(EntityId id) const noexcept { return entities_.has(id); }

  /// Creates a component in-place for the given entity-id.
  /// \tparam T The type of the component.
  /// \tparam Ts The types of the constructor arguments.
  /// \param entity The entity-id for which we create the component.
  /// \param args The arguments to the component constructor.
  template <Component T, typename... Ts>
  void emplaceComponent(EntityId entity, Ts&&...args);

  /// Sets a component for the given entity-id.
  /// \tparam T The type of the component.
  /// \param entity The entity-id for which we set the component.
  /// \param value The value of the component.
  template <Component T>
  void setComponent(EntityId entity, const T& value);

  /// Removes a component from the given entity.
  /// \tparam T The type of the component.
  /// \param entity The entity-id from which we remove the component.
  template <Component T>
  bool removeComponent(EntityId entity);

  /// Checks if an entity has a component of the given type.
  /// \param entity The entity-id to check.
  /// \tparam T The type of the component.
  /// \return True if the entity has the component, false otherwise.
  template <Component T>
  bool hasComponent(EntityId entity) const noexcept;

  /// Gets a component for the given entity-id.
  /// \tparam T The type of the component.
  /// \param entity The entity-id from which we get the component.
  /// \return A pointer to the component if it exists, nullptr otherwise.
  template <Component T>
  T *getComponent(EntityId entity);

  /// Gets a component for the given entity-id.
  /// \tparam T The type of the component.
  /// \param entity The entity-id from which we get the component.
  /// \return A pointer to the component if it exists, nullptr otherwise.
  template <Component T>
  const T *getComponent(EntityId entity) const;

  /// Returns a view over the entities with the given components and filters.
  /// \tparam Components The components constituting the view.
  /// \tparam Filters The filters to apply to the view.
  /// \param filters The components to filter from the view.
  /// \return An iterable view, returning a tuple of the requested components.
  template <Component... Components,
            internal::FilterListLike Filters = internal::FilterList<>>
  View<Components...> view(Filters = filter<>);

  /// Returns a view over the entities with the given components and filters.
  /// \tparam Components The components constituting the view.
  /// \tparam Filters The filters to apply to the view.
  /// \param filters The components to filter from the view.
  /// \return An iterable view, returning a tuple of the requested components.
  template <Component... Components,
            internal::FilterListLike Filters = internal::FilterList<>>
  View<Components...> view(Filters = filter<>) const;

private:
  internal::Entities   entities_ = {};
  internal::Components components_ = {};
};

}  // namespace bad

#endif  // BADECS_REGISTRY_H
