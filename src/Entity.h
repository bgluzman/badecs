#pragma once

#include "Common.h"
#include "ComponentRegistry.h"

#include <gsl/gsl>
#include <optional>

namespace bad {

class Entity {
public:
  explicit Entity(EntityId id, gsl::not_null<ComponentRegistry *> components);
  ~Entity() noexcept = default;
  Entity(Entity&&) noexcept = default;
  Entity& operator=(Entity&&) noexcept = default;
  Entity(const Entity&) = delete;
  Entity& operator=(const Entity&) = delete;

  [[nodiscard]] EntityId getId() const noexcept { return id_; }

  template <Component T, typename... Ts>
  void emplace(Ts&&...args);
  template <Component T>
  T *get();
  template <Component T>
  void set(const T& value);

private:
  EntityId                           id_;
  gsl::not_null<ComponentRegistry *> components_;
};

inline Entity::Entity(EntityId                           id,
                      gsl::not_null<ComponentRegistry *> components)
    : id_(id), components_(components) {}

template <Component T, typename... Ts>
void Entity::emplace(Ts&&...args) {
  components_->emplace<T, Ts...>(id_, std::forward<Ts>(args)...);
}

template <Component T>
T *Entity::get() {
  return components_->get<T>(id_);
}

template <Component T>
void Entity::set(const T& value) {
  components_->set(id_, value);
}

}  // namespace bad