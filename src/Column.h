#pragma once

#include "Common.h"

#include <any>
#include <gsl/gsl>
#include <map>
#include <memory>
#include <ranges>
#include <vector>

namespace bad {

struct Column {
public:
  template <Component T, typename... Ts>
  void emplace(EntityId entityId, Ts&&...args);
  template <Component T>
  void set(EntityId entityId, const T& value);
  template <Component T>
  bool has(EntityId entityId) const noexcept;
  template <Component T>
  T *get(EntityId entityId);

  [[nodiscard]] decltype(auto) getEntityIds() const;

private:
  // XXX: std::map's pointer/reference invalidation semantics here are
  //  incredibly important since get() returns a pointer into the map. If
  //  another map implementation is chosen, it is possible that get() should
  //  have different semantics OR the values in the map should be pointers.
  std::map<EntityId, std::any> components_ = {};
};

template <Component T, typename... Ts>
void Column::emplace(EntityId entityId, Ts&&...args) {
  components_[entityId] =
      std::any(std::in_place_type<T>, std::forward<Ts>(args)...);
}

template <Component T>
void Column::set(EntityId entityId, const T& value) {
  components_[entityId] = value;
}

template <Component T>
bool Column::has(EntityId entityId) const noexcept {
  return components_.contains(entityId);
}

template <Component T>
T *Column::get(EntityId entityId) {
  if (auto it = components_.find(entityId); it != components_.end()) {
    return &std::any_cast<T&>(it->second);
  } else {
    return nullptr;
  }
}

inline decltype(auto) Column::getEntityIds() const {
  return components_ | std::views::keys;
}

}  // namespace bad
