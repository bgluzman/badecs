#pragma once

#include <any>
#include <cstdint>
#include <gsl/gsl>
#include <type_traits>
#include <unordered_map>

namespace bad {

template <typename T>
concept Component = std::is_trivially_copyable_v<T>;

using EntityId = std::uint32_t;
using ComponentId = std::uint32_t;

static constexpr EntityId kInvalidEntityId = 0ULL;

class EntityStorage {};

// TODO (bgluzman): totally redo this so it's a dynamic storage
struct Column {
  std::unordered_map<EntityId, std::unique_ptr<std::any>> components = {};
};

template <Component T>
class ComponentView {
public:
  ComponentView(Column *column, EntityId entity_id)
      : column_(column), entity_id_(entity_id) {}

  operator bool() const noexcept {  // NOLINT(google-explicit-constructor)
    return column_ &&
           column_->components.find(entity_id_) != column_->components.end();
  }

  T& operator*() {
    return std::any_cast<T&>(*column_->components.find(entity_id_)->second);
  }

private:
  Column  *column_;
  EntityId entity_id_;
};
class ComponentStorage {
public:
  template <Component T>
  static ComponentId getComponentId();

  template <Component T, typename... Ts>
  void add(EntityId entityId, Ts&&...args);
  template <Component T>
  ComponentView<T> get(EntityId entityId);
  template <Component T>
  void set(EntityId entityId, const T& value);

private:
  template <Component T>
  Column& getColumn(EntityId entityId);

  static inline ComponentId kComponentIdCounter = 1;
  // TODO (bgluzman): use a flat array w/ free list instead?
  std::unordered_map<ComponentId, std::unique_ptr<Column>> columns_ = {};
};

class SystemStorage {};

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
  ComponentView<T> get();
  template <Component T>
  void set(const T& value);
  template <Component T>
  void remove();

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

template <Component T, typename... Ts>
void ComponentStorage::add(EntityId entityId, Ts&&...args) {
  Column& col = getColumn<T>(entityId);
  col.components[entityId] = std::make_unique<std::any>(
      std::in_place_type<T>, std::forward<Ts>(args)...);
}

template <Component T>
ComponentView<T> ComponentStorage::get(EntityId entityId) {
  ComponentId componentId = getComponentId<T>();
  if (auto it = columns_.find(componentId); it != columns_.end()) {
    return ComponentView<T>(it->second.get(), entityId);
  } else {
    return ComponentView<T>(nullptr, kInvalidEntityId);
  }
}

template <Component T>
void ComponentStorage::set(EntityId entityId, const T& value) {
  // TODO (bgluzman): DRY! most of implementation shared w/ add
  Column& col = getColumn<T>(entityId);
  col.components[entityId] = std::make_unique<std::any>(value);
}

template <Component T>
inline Column& ComponentStorage::getColumn(EntityId entityId) {
  ComponentId              componentId = getComponentId<T>();
  std::unique_ptr<Column>& col = columns_[componentId];
  if (!col) {
    // TODO (bgluzman): use a pool allocator?
    col = std::make_unique<Column>();
  }
  return *col;
}

inline Entity::Entity(EntityId id, gsl::not_null<ComponentStorage *> components)
    : id_(id), components_(components) {}

template <Component T, typename... Ts>
void Entity::add(Ts&&...args) {
  components_->add<T, Ts...>(id_, std::forward<Ts>(args)...);
}

template <Component T>
ComponentView<T> Entity::get() {
  return components_->get<T>(id_);
}

template <Component T>
void Entity::set(const T& value) {
  components_->set(id_, value);
}

template <Component T>
void Entity::remove() {}

}  // namespace bad