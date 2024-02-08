#pragma once

#include "Common.h"
#include "ComponentManager.h"

#include <gsl/gsl>
#include <optional>

namespace bad {

class EntityHandle {
public:
  explicit EntityHandle(EntityId                          id,
                        gsl::not_null<ComponentManager *> components);
  ~EntityHandle() noexcept = default;
  EntityHandle(EntityHandle&&) noexcept = default;
  EntityHandle& operator=(EntityHandle&&) noexcept = default;
  EntityHandle(const EntityHandle&) = delete;
  EntityHandle& operator=(const EntityHandle&) = delete;

  [[nodiscard]] EntityId getId() const noexcept { return id_; }

  template <Component T, typename... Ts>
  void add(Ts&&...args);
  template <Component T>
  std::optional<gsl::not_null<T *>> get();
  template <Component T>
  void set(const T& value);

private:
  EntityId                          id_;
  gsl::not_null<ComponentManager *> components_;
};

inline EntityHandle::EntityHandle(EntityId                          id,
                                  gsl::not_null<ComponentManager *> components)
    : id_(id), components_(components) {}

template <Component T, typename... Ts>
void EntityHandle::add(Ts&&...args) {
  components_->add<T, Ts...>(id_, std::forward<Ts>(args)...);
}

template <Component T>
std::optional<gsl::not_null<T *>> EntityHandle::get() {
  return components_->get<T>(id_);
}

template <Component T>
void EntityHandle::set(const T& value) {
  components_->set(id_, value);
}

}  // namespace bad