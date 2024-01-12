#pragma once

#include <cstdint>
#include <gsl/gsl>
#include <type_traits>
#include <unordered_map>

namespace bad {

template <typename T>
concept Component = std::is_trivially_copyable_v<T>;

using EntityId = std::uint32_t;
using ComponentId = std::uint32_t;

class EntityStorage {};
class ComponentStorage {
public:
  template <Component T> static ComponentId getComponentId();

  template <Component T, typename... Ts>
  void                        add(EntityId entityId, Ts&&...args);
  template <Component T> bool has(EntityId entityId);
  template <Component T> T&   get(EntityId entityId);
  template <Component T> void set(EntityId entityId, const T& value);

private:
  static inline ComponentId kComponentIdCounter = 1;

  // TODO (bgluzman): totally redo this so it's a dynamic storage
  struct Column {
    static inline constexpr std::size_t kBufSize = 1024ULL;
    std::byte                           buf[kBufSize];
    std::size_t                         pos = 0ULL;

    std::unordered_map<EntityId, std::size_t> index = {};
  };

  // TODO (bgluzman): use a flat array w/ free list instead?
  std::unordered_map<ComponentId, Column> columns_ = {};
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

  template <Component T, typename... Ts> void add(Ts&&...args);
  template <Component T> bool                 has();
  template <Component T> T&                   get();
  template <Component T> void                 set(const T& value);
  template <Component T> void                 remove();

private:
  EntityId                          id_;
  gsl::not_null<ComponentStorage *> components_;
};

class World {};

template <Component T> ComponentId ComponentStorage::getComponentId() {
  static ComponentId id = kComponentIdCounter++;
  return id;
}

template <Component T, typename... Ts>
void ComponentStorage::add(EntityId entityId, Ts&&...args) {
  // TODOD (bgluzman): DRY!
  static constexpr std::size_t kComponentSize = sizeof(T);
  ComponentId                  componentId = getComponentId<T>();
  Column&                      col = columns_[componentId];

  // TODO (bgluzman): all logic here should be offloaded to Column defn
  if (col.pos + kComponentSize > Column::kBufSize) {
    // TODO (bgluzman): proper error-handling
    throw std::runtime_error{"column overflow"};
  }
  new (&col.buf[col.pos]) T(std::forward<Ts>(args)...);
  col.index[entityId] = col.pos;
  col.pos += kComponentSize;
}

template <Component T> bool ComponentStorage::has(EntityId entityId) {
  ComponentId componentId = getComponentId<T>();
  if (auto columns_it = columns_.find(componentId);
      columns_it != columns_.end()) {
    Column& column = columns_it->second;
    return column.index.contains(entityId);
  }
  return false;
}

template <Component T> T& ComponentStorage::get(EntityId entityId) {
  // TODO (bgluzman): make more efficient by utilizing U.B.?
  ComponentId componentId = getComponentId<T>();
  if (auto columns_it = columns_.find(componentId);
      columns_it != columns_.end()) {
    Column& column = columns_it->second;
    if (auto index_it = column.index.find(entityId);
        index_it != column.index.end()) {
      std::size_t pos = index_it->second;
      return *std::launder<T>(reinterpret_cast<T *>(&column.buf[pos]));
    }
  }
  throw std::runtime_error{"bad access"};
}

template <Component T>
void ComponentStorage::set(EntityId entityId, const T& value) {
  static constexpr std::size_t kComponentSize = sizeof(T);
  ComponentId                  componentId = getComponentId<T>();
  Column&                      col = columns_[componentId];

  // TODO (bgluzman): all logic here should be offloaded to Column defn
  if (col.pos + kComponentSize > Column::kBufSize) {
    // TODO (bgluzman): proper error-handling
    throw std::runtime_error{"column overflow"};
  }
  std::memcpy(&col.buf[col.pos], &value, kComponentSize);
  col.index[entityId] = col.pos;
  col.pos += kComponentSize;
}

inline Entity::Entity(EntityId id, gsl::not_null<ComponentStorage *> components)
    : id_(id), components_(components) {}

template <Component T, typename... Ts> void Entity::add(Ts&&...args) {
  components_->add<T, Ts...>(id_, std::forward<Ts>(args)...);
}

template <Component T> bool Entity::has() { return components_->has<T>(id_); }

template <Component T> T& Entity::get() { return components_->get<T>(id_); }

template <Component T> void Entity::set(const T& value) {
  components_->set(id_, value);
}

template <Component T> void Entity::remove() {}

}  // namespace bad