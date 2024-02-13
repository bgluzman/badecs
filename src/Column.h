#pragma once

#include "Common.h"

#include <algorithm>
#include <any>
#include <gsl/gsl>
#include <memory>
#include <unordered_map>
#include <vector>

namespace bad {

struct Column {
public:
  [[nodiscard]] std::vector<EntityId> getEntityIds() const;

  template <Component T, typename... Ts>
  void emplace(EntityId entityId, Ts&&...args);
  template <Component T>
  T *get(EntityId entityId);
  template <Component T>
  void set(EntityId entityId, const T& value);

private:
  // XXX: std::unordered_map's pointer/reference invalidation semantics here are
  //  incredibly important since get() returns a pointer into the map. If
  //  another map implementation is chosen, get() should have different
  //  semantics OR the values in the map should be pointers. See:
  //  https://en.cppreference.com/w/cpp/container/unordered_map#Iterator_invalidation
  std::unordered_map<EntityId, std::any> components_ = {};
};

template <Component T, typename... Ts>
void Column::emplace(EntityId entityId, Ts&&...args) {
  components_[entityId] =
      std::any(std::in_place_type<T>, std::forward<Ts>(args)...);
}

template <Component T>
T *Column::get(EntityId entityId) {
  if (auto it = components_.find(entityId); it != components_.end()) {
    return &std::any_cast<T&>(it->second);
  } else {
    return nullptr;
  }
}

template <Component T>
void Column::set(EntityId entityId, const T& value) {
  components_[entityId] = value;
}

}  // namespace bad
