#pragma once

#include "Column.h"

#include <any>
#include <gsl/gsl>

namespace bad {

class ComponentRegistry {
public:
  template <Component T>
  static ComponentId getComponentId();

  template <Component T, typename... Ts>
  void emplace(EntityId entityId, Ts&&...args);
  template <Component T>
  T *get(EntityId entityId);
  template <Component T>
  void set(EntityId entityId, const T& value);

  template <Component Arg, Component... Args>
  std::vector<EntityId> getQueryComponents();

private:
  // TODO (bgluzman): evaluate const-ness so getQueryComponents can be const
  template <Component T>
  Column& getColumn();

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
void ComponentRegistry::emplace(EntityId entityId, Ts&&...args) {
  Column& col = getColumn<T>();
  col.emplace<T>(entityId, std::forward<Ts>(args)...);
}

template <Component T>
T *ComponentRegistry::get(EntityId entityId) {
  ComponentId componentId = getComponentId<T>();
  if (auto it = columns_.find(componentId); it != columns_.end()) {
    return it->second->get<T>(entityId);
  } else {
    return nullptr;
  }
}

template <Component T>
void ComponentRegistry::set(EntityId entityId, const T& value) {
  Column& col = getColumn<T>();
  col.set(entityId, value);
}

template <Component Arg, Component... Args>
std::vector<EntityId> ComponentRegistry::getQueryComponents() {
  if constexpr (sizeof...(Args) == 0) {
    return getColumn<Arg>().getEntityIds();
  } else {
    auto argQueryComponents = getColumn<Arg>().getEntityIds();
    auto argsQueryComponents = getQueryComponents<Args...>();

    std::vector<EntityId> result;
    std::ranges::set_intersection(argQueryComponents, argsQueryComponents,
                                  std::back_inserter(result));
    return result;
  }
}

}  // namespace bad