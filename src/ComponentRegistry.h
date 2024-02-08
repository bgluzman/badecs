#pragma once

#include "Column.h"

#include <any>
#include <gsl/gsl>

namespace bad {

class ComponentRegistry {
public:
  template <Component T>
  static ComponentId getComponentId();
  template <Component T>
  Column& getColumn();

  template <Component T, typename... Ts>
  void add(EntityId entityId, Ts&&...args);
  template <Component T>
  std::optional<gsl::not_null<T *>> get(EntityId entityId);
  template <Component T>
  T& getUnchecked(EntityId entityId);
  template <Component T>
  void set(EntityId entityId, const T& value);

private:
  // TODO (bgluzman): should not be static!
  static inline ComponentId kComponentIdCounter = 1;
  std::unordered_map<ComponentId, std::unique_ptr<Column>> columns_ = {};
};

template <Component T>
ComponentId ComponentRegistry::getComponentId() {
  static ComponentId id = kComponentIdCounter++;
  return id;
}

template <Component T>
inline Column& ComponentRegistry::getColumn() {
  ComponentId              componentId = getComponentId<T>();
  std::unique_ptr<Column>& col = columns_[componentId];
  if (!col) {
    col = std::make_unique<Column>();
  }
  return *col;
}

template <Component T, typename... Ts>
void ComponentRegistry::add(EntityId entityId, Ts&&...args) {
  Column& col = getColumn<T>();
  col.components[entityId] = std::make_unique<std::any>(
      std::in_place_type<T>, std::forward<Ts>(args)...);
}

template <Component T>
std::optional<gsl::not_null<T *>> ComponentRegistry::get(EntityId entityId) {
  ComponentId componentId = getComponentId<T>();
  if (auto it = columns_.find(componentId); it != columns_.end()) {
    return std::any_cast<T>(it->second->components[entityId].get());
  } else {
    return std::nullopt;
  }
}

template <Component T>
T& ComponentRegistry::getUnchecked(bad::EntityId entityId) {
  ComponentId componentId = getComponentId<T>();
  return *std::any_cast<T>(
      columns_.at(componentId)->components.at(entityId).get());
}

template <Component T>
void ComponentRegistry::set(EntityId entityId, const T& value) {
  Column& col = getColumn<T>();
  col.components[entityId] = std::make_unique<std::any>(value);
}

}  // namespace bad