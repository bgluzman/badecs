#pragma once

#include <any>
#include <cstdint>
#include <gsl/gsl>
#include <type_traits>
#include <unordered_map>

namespace bad {

// CopyConstructible is a requirement for std::any
template <typename T>
concept Component = std::is_copy_constructible_v<T>;

using EntityId = std::uint32_t;
using ComponentId = std::uint32_t;

struct Column {
  std::unordered_map<EntityId, std::unique_ptr<std::any>> components = {};
};

class ComponentStorage {
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
  void set(EntityId entityId, const T& value);

private:
  static inline ComponentId kComponentIdCounter = 1;
  std::unordered_map<ComponentId, std::unique_ptr<Column>> columns_ = {};
};

class Entity {

public:
  explicit Entity(EntityId id, gsl::not_null<ComponentStorage *> components);
  ~Entity() noexcept = default;
  Entity(const Entity&) = delete;
  Entity(Entity&&) noexcept = delete;
  Entity& operator=(const Entity&) = delete;
  Entity& operator=(Entity&&) noexcept = delete;

  template <Component T, typename... Ts>
  void add(Ts&&...args);
  template <Component T>
  std::optional<gsl::not_null<T *>> get();
  template <Component T>
  void set(const T& value);

private:
  EntityId                          id_;
  gsl::not_null<ComponentStorage *> components_;
};

class World {};

template <Component T>
ComponentId ComponentStorage::getComponentId() {
  static ComponentId id = kComponentIdCounter++;
  return id;
}

template <Component T>
inline Column& ComponentStorage::getColumn() {
  ComponentId              componentId = getComponentId<T>();
  std::unique_ptr<Column>& col = columns_[componentId];
  if (!col) {
    col = std::make_unique<Column>();
  }
  return *col;
}

template <Component T, typename... Ts>
void ComponentStorage::add(EntityId entityId, Ts&&...args) {
  Column& col = getColumn<T>();
  col.components[entityId] = std::make_unique<std::any>(
      std::in_place_type<T>, std::forward<Ts>(args)...);
}

template <Component T>
std::optional<gsl::not_null<T *>> ComponentStorage::get(EntityId entityId) {
  ComponentId componentId = getComponentId<T>();
  if (auto it = columns_.find(componentId); it != columns_.end()) {
    return std::any_cast<T>(it->second->components[entityId].get());
  } else {
    return std::nullopt;
  }
}

template <Component T>
void ComponentStorage::set(EntityId entityId, const T& value) {
  Column& col = getColumn<T>();
  col.components[entityId] = std::make_unique<std::any>(value);
}

inline Entity::Entity(EntityId id, gsl::not_null<ComponentStorage *> components)
    : id_(id), components_(components) {}

template <Component T, typename... Ts>
void Entity::add(Ts&&...args) {
  components_->add<T, Ts...>(id_, std::forward<Ts>(args)...);
}

template <Component T>
std::optional<gsl::not_null<T *>> Entity::get() {
  return components_->get<T>(id_);
}

template <Component T>
void Entity::set(const T& value) {
  components_->set(id_, value);
}

}  // namespace bad