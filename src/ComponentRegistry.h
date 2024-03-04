#pragma once

#include "Column.h"

#include <any>
#include <gsl/gsl>
#include <ranges>
#include <set>
#include <unordered_map>

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
  bool remove(EntityId entity);
  bool remove(EntityId entity, ComponentId component);
  template <Component T>
  [[nodiscard]] bool has(EntityId entityId) const noexcept;
  [[nodiscard]] bool has(EntityId    entityId,
                         ComponentId component) const noexcept;
  template <Component T>
  [[nodiscard]] T *get(EntityId entityId);

  template <Component Arg>
  [[nodiscard]] decltype(auto) entitiesWithComponent() const;

private:
  template <Component T>
  [[nodiscard]] Column *getColumn();
  [[nodiscard]] Column *getColumn(ComponentId id);
  template <Component T>
  [[nodiscard]] const Column *getColumn() const;
  [[nodiscard]] const Column *getColumn(ComponentId id) const;
  template <Component T>
  Column& getOrCreateColumn();

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

inline Column *ComponentRegistry::getColumn(ComponentId id) {
  return const_cast<Column *>(
      const_cast<const ComponentRegistry *>(this)->getColumn(id));
}

template <Component T>
const Column *ComponentRegistry::getColumn() const {
  return getColumn(getComponentId<T>());
}

inline const Column *ComponentRegistry::getColumn(ComponentId id) const {
  if (auto it = columns_.find(id); it != columns_.end()) {
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
void ComponentRegistry::set(EntityId entityId, const T& value) {
  Column& col = getOrCreateColumn<T>();
  col.set(entityId, value);
}

template <Component T>
bool ComponentRegistry::remove(EntityId entity) {
  return remove(entity, getComponentId<T>());
}

inline bool ComponentRegistry::remove(EntityId entity, ComponentId component) {
  if (Column *col = getColumn(component); col) {
    return col->remove(entity);
  }
  return false;
}

template <Component T>
bool ComponentRegistry::has(EntityId entity) const noexcept {
  return has(entity, getComponentId<T>());
}

inline bool ComponentRegistry::has(EntityId    entity,
                                   ComponentId component) const noexcept {
  const Column *col = getColumn(component);
  return col && col->has(entity);
}

template <Component T>
T *ComponentRegistry::get(EntityId entityId) {
  if (Column *col = getColumn<T>(); col) {
    return std::any_cast<T>(col->get(entityId));
  }
  return nullptr;
}

template <Component Arg>
decltype(auto) ComponentRegistry::entitiesWithComponent() const {
  const Column *col = getColumn<Arg>();
  if (!col) {
    const static std::map<EntityId, std::any> kEmptySet;
    return kEmptySet | std::views::keys;
  }
  return col->getEntityIds();
}

}  // namespace bad