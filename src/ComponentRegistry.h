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
  void set(EntityId entityId, const T& value);
  template <Component T>
  bool has(EntityId entityId) const noexcept;
  template <Component T>
  T *get(EntityId entityId);

  template <Component Arg, Component... Args>
  [[nodiscard]] std::vector<EntityId> getQueryComponents() const;

private:
  template <Component T>
  [[nodiscard]] Column *getColumn();
  template <Component T>
  [[nodiscard]] const Column *getColumn() const;
  template <Component T>
  Column& getOrCreateColumn();

  // TODO (bgluzman): should not be static!
  static inline ComponentId kComponentIdCounter = 1;
  // XXX: std::unordered_map's semantics are important here! See similar note
  //  in Column.h.
  std::unordered_map<ComponentId, Column> columns_ = {};
};

template <Component T>
ComponentId ComponentRegistry::getComponentId() {
  static ComponentId id = kComponentIdCounter++;
  return id;
}

template <Component T>
Column *ComponentRegistry::getColumn() {
  return const_cast<Column *>(
      const_cast<const ComponentRegistry *>(this)->getColumn<T>());
}

template <Component T>
const Column *ComponentRegistry::getColumn() const {
  ComponentId componentId = getComponentId<T>();
  if (auto it = columns_.find(componentId); it != columns_.end()) {
    return &it->second;
  }
  return nullptr;
}

template <Component T>
Column& ComponentRegistry::getOrCreateColumn() {
  if (Column *col = getColumn<T>(); col) {
    return *col;
  }
  ComponentId componentId = getComponentId<T>();
  return columns_.emplace(componentId, Column{}).first->second;
}

template <Component T, typename... Ts>
void ComponentRegistry::emplace(EntityId entityId, Ts&&...args) {
  Column& col = getOrCreateColumn<T>();
  col.emplace<T>(entityId, std::forward<Ts>(args)...);
}

template <Component T>
bool ComponentRegistry::has(EntityId entityId) const noexcept {
  const Column *col = getColumn<T>();
  return col && col->has<T>(entityId);
}

template <Component T>
void ComponentRegistry::set(EntityId entityId, const T& value) {
  Column& col = getOrCreateColumn<T>();
  col.set(entityId, value);
}

template <Component T>
T *ComponentRegistry::get(EntityId entityId) {
  if (Column *col = getColumn<T>(); col) {
    return col->get<T>(entityId);
  }
  return nullptr;
}

template <Component Arg, Component... Args>
std::vector<EntityId> ComponentRegistry::getQueryComponents() const {
  const Column *argQueryColumn = getColumn<Arg>();
  if (!argQueryColumn) {
    // TODO (bgluzman): print a diagnostic warning?
    return {};
  }

  if constexpr (sizeof...(Args) == 0) {
    return argQueryColumn->getEntityIds();
  } else {
    auto argQueryComponents = argQueryColumn->getEntityIds();
    auto argsQueryComponents = getQueryComponents<Args...>();

    std::vector<EntityId> result;
    std::ranges::set_intersection(argQueryComponents, argsQueryComponents,
                                  std::back_inserter(result));
    return result;
  }
}

}  // namespace bad