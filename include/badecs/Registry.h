#ifndef BADECS_REGISTRY_H
#define BADECS_REGISTRY_H

#include <badecs/Common.h>
#include <badecs/Filter.h>
#include <badecs/internal/Components.h>
#include <badecs/internal/Entities.h>

namespace bad {

/// @brief The top-level registry of entity-component relationships.
///
/// Maintains entities and their constituent components. Allows creation,
/// modification, and deletion of entities/components along with querying
/// for entities with specific components.
class Registry {

public:
  // TODO (bgluzman): docstring
  EntityId createEntity();

  // TODO (bgluzman): docstring
  EntityId reserveEntity();

  // TODO (bgluzman): docstring
  void instantiateEntity(EntityId id);

  // TODO (bgluzman): docstring
  bool destroyEntity(EntityId id);

  // TODO (bgluzman): docstring
  bool hasEntity(EntityId id) const noexcept;

  // TODO (bgluzman): docstring
  template <Component T, typename... Ts>
  void emplaceComponent(EntityId entity, Ts&&...args);

  // TODO (bgluzman): docstring
  template <Component T>
  void setComponent(EntityId entity, const T& value);

  // TODO (bgluzman): docstring
  template <Component T>
  bool removeComponent(EntityId entity);

  // TODO (bgluzman): docstring
  template <Component T>
  bool hasComponent(EntityId entity) const noexcept;

  // TODO (bgluzman): docstring
  template <Component T>
  T *getComponent(EntityId entity);

  // TODO (bgluzman): docstring
  template <Component T>
  const T *getComponent(EntityId entity) const;

  // TODO (bgluzman): docstring
  template <Component... Components,
            internal::FilterListLike Filters = internal::FilterList<>>
  auto view(Filters = filter<>);

  // TODO (bgluzman): docstring
  template <Component... Components,
            internal::FilterListLike Filters = internal::FilterList<>>
  auto view(Filters = filter<>) const;

private:
  internal::Entities   entities_ = {};
  internal::Components components_ = {};
};

}  // namespace bad

#endif  // BADECS_REGISTRY_H
