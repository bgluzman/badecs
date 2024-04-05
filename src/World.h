#pragma once

#include "Common.h"
#include "ComponentRegistry.h"
#include "EntityRegistry.h"

#include <any>
#include <cstdint>
#include <gsl/gsl>
#include <iostream>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bad {

/// @brief This is a test of Doxygen.
class World {
public:
  EntityId           create();
  EntityId           reserve();
  void               instantiate(EntityId id);
  bool               destroy(EntityId id);
  [[nodiscard]] bool has(EntityId id) const noexcept;

  template <Component T, typename... Ts>
  void emplaceComponent(EntityId entity, Ts&&...args);
  template <Component T>
  void setComponent(EntityId entity, const T& value);
  template <Component T>
  bool removeComponent(EntityId entity);
  template <Component T>
  [[nodiscard]] bool hasComponent(EntityId entity) const noexcept;
  template <Component T>
  [[nodiscard]] T *getComponent(EntityId entity);
  template <Component T>
  [[nodiscard]] const T *getComponent(EntityId entity) const;

  template <Component... Components, typename Filters = Filter<>>
  [[nodiscard]] auto view(Filters = filter<>);
  template <Component... Components, typename Filters = Filter<>>
  [[nodiscard]] auto view(Filters = filter<>) const;

private:
  std::unique_ptr<EntityRegistry> entities_ =
      std::make_unique<EntityRegistry>();
  std::unique_ptr<ComponentRegistry> components_ =
      std::make_unique<ComponentRegistry>();
};

inline EntityId World::create() { return entities_->add(); }

inline EntityId World::reserve() { return entities_->reserve(); }

inline void World::instantiate(bad::EntityId id) { entities_->instantiate(id); }

inline bool World::destroy(EntityId id) {
  if (auto components = entities_->remove(id); components.has_value()) {
    for (ComponentId componentId : *components) {
      // XXX: Cannot call removeComponent since the entity is already removed.
      components_->remove(id, componentId);
    }
    return true;
  }
  return false;
}

inline bool World::has(EntityId id) const noexcept {
  return entities_->has(id);
}

template <Component T, typename... Ts>
void World::emplaceComponent(EntityId entity, Ts&&...args) {
  components_->emplace<T>(entity, std::forward<Ts>(args)...);
  entities_->addComponent(entity, components_->getComponentId<T>());
}

template <Component T>
void World::setComponent(EntityId entity, const T& value) {
  components_->set(entity, value);
  entities_->addComponent(entity, components_->getComponentId<T>());
}

template <Component T>
bool World::removeComponent(EntityId entity) {
  ComponentId component = components_->getComponentId<T>();
  entities_->removeComponent(entity, component);
  return components_->remove(entity, component);
}

template <Component T>
bool World::hasComponent(EntityId entity) const noexcept {
  return components_->has<T>(entity);
}

template <Component T>
T *World::getComponent(EntityId entity) {
  return components_->get<T>(entity);
}

template <Component T>
const T *World::getComponent(EntityId entity) const {
  return components_->get<T>(entity);
}

template <Component... Components, typename Filters>
auto World::view(Filters filters) {
  return components_->view<Components...>(filters);
}

template <Component... Components, typename Filters>
auto World::view(Filters filters) const {
  return components_->view<Components...>(filters);
}

}  // namespace bad