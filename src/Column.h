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
  std::vector<EntityId> getEntityIds();

  template <Component T, typename... Ts>
  void add(EntityId entityId, Ts&&...args);
  template <Component T>
  gsl::not_null<T *> get(EntityId entityId);
  template <Component T>
  void set(EntityId entityId, const T& value);

private:
  std::unordered_map<EntityId, std::unique_ptr<std::any>> components_ = {};
};

template <Component T, typename... Ts>
void Column::add(EntityId entityId, Ts&&...args) {
  components_[entityId] = std::make_unique<std::any>(std::in_place_type<T>,
                                                     std::forward<Ts>(args)...);
}

template <Component T>
gsl::not_null<T *> Column::get(EntityId entityId) {
  return std::any_cast<T>(components_[entityId].get());
}

template <Component T>
void Column::set(EntityId entityId, const T& value) {
  components_[entityId] = std::make_unique<std::any>(value);
}

}  // namespace bad
