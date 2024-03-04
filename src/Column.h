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
  void                    emplace(EntityId entityId, Ts&&...args);
  void                    set(EntityId entityId, std::any value);
  bool                    remove(EntityId entityId);
  [[nodiscard]] bool      has(EntityId entityId) const noexcept;
  [[nodiscard]] std::any *get(EntityId entityId);

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

inline void Column::set(EntityId entityId, std::any value) {
  components_[entityId] = std::move(value);
}

inline bool Column::remove(EntityId entityId) {
  return components_.erase(entityId) > 0;
}

inline bool Column::has(EntityId entityId) const noexcept {
  return components_.contains(entityId);
}

inline std::any *Column::get(EntityId entityId) {
  if (auto it = components_.find(entityId); it != components_.end()) {
    return &it->second;
  } else {
    return nullptr;
  }
}

inline decltype(auto) Column::getEntityIds() const {
  return components_ | std::views::keys;
}

}  // namespace bad
