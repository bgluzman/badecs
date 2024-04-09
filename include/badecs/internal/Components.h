#ifndef BADECS_INTERNAL_COMPONENTS_H
#define BADECS_INTERNAL_COMPONENTS_H

#include <any>
#include <badecs/Common.h>
#include <badecs/internal/Column.h>
#include <ranges>

namespace bad::internal {

/// \brief Storage of components in our registry.
///
/// This class stores components together in a `Column` which are indexed by
/// their `ComponentId`.
class Components {
public:
  // TODO (bgluzman): docstring
  template <Component T, typename... Ts>
  void emplace(EntityId entityId, Ts&&...args);

  // TODO (bgluzman): docstring
  template <Component T>
  void set(EntityId entity, const T& value);

  // TODO (bgluzman): docstring
  template <Component T>
  bool remove(EntityId entity);

  // TODO (bgluzman): docstring
  template <std::ranges::range T>
  bool removeAll(EntityId entity, const T& componentsToRemove);

  // TODO (bgluzman): docstring
  template <typename T>
  bool has(EntityId entityId) const noexcept;

  // TODO (bgluzman): docstring
  template <Component T>
  T *get(EntityId entityId);

  // TODO (bgluzman): docstring
  template <Component T>
  const T *get(EntityId entityId) const;

private:
  std::map<ComponentId, Column> components_;
};

}  // namespace bad::internal

#endif