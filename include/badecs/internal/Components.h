#ifndef BADECS_INTERNAL_COMPONENTS_H
#define BADECS_INTERNAL_COMPONENTS_H

#include <badecs/Common.h>
#include <badecs/internal/Column.h>

#include <any>

namespace bad::internal {

// TODO (bgluzman): docstring
class Components {
public:
  // TODO (bgluzman): docstring
  template <Component T, typename... Ts>
  void emplace(EntityId entityId, Ts &&...args);

  // TODO (bgluzman): docstring
  void set(EntityId entity, ComponentId component, std::any value);

  // TODO (bgluzman): docstring
  bool remove(EntityId entity, ComponentId component);

  // TODO (bgluzman): docstring
  bool has(EntityId entityId, ComponentId component) const noexcept;

  // TODO (bgluzman): docstring
  std::any *get(EntityId entityId, ComponentId component);

  // TODO (bgluzman): docstring
  const std::any *get(EntityId entityId, ComponentId component) const;

private:
  std::map<ComponentId, Column> components_;
};

} // namespace bad::internal

#endif