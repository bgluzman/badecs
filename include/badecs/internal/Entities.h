#ifndef BADECS_INTERNAL_ENTITIES_H
#define BADECS_INTERNAL_ENTITIES_H

#include <badecs/Common.h>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace bad::internal {

// TODO (bgluzman): docstring
class Entities {

public:
  // TODO (bgluzman): docstring
  EntityId reserve();

  // TODO (bgluzman): docstring
  void instantiate(EntityId id);

  // TODO (bgluzman): docstring
  std::optional<std::unordered_set<ComponentId>> remove(EntityId id);

  // TODO (bgluzman): docstring
  bool has(EntityId id) const noexcept;

  // TODO (bgluzman): docstring
  void addComponent(EntityId entity, ComponentId component);

  // TODO (bgluzman): docstring
  void removeComponent(EntityId entity, ComponentId component);

private:
  EntityId entityCounter_ = 1;
  std::unordered_map<EntityId, std::unordered_set<ComponentId>> entities_ = {};
};

}  // namespace bad::internal

#endif
