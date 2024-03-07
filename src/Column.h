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
  void                          emplace(EntityId entityId, Ts&&...args);
  void                          set(EntityId entityId, std::any value);
  bool                          remove(EntityId entityId);
  [[nodiscard]] bool            has(EntityId entityId) const noexcept;
  [[nodiscard]] std::any       *get(EntityId entityId);
  [[nodiscard]] const std::any *get(EntityId entityId) const;

  [[nodiscard]] std::size_t size() const noexcept;

  [[nodiscard]] decltype(auto) getEntityIds() const;

  auto                     begin() noexcept { return components_.begin(); }
  auto                     end() noexcept { return components_.end(); }
  [[nodiscard]] const auto cbegin() const noexcept {
    return components_.cbegin();
  }
  [[nodiscard]] const auto cend() const noexcept { return components_.cend(); }

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
  return const_cast<std::any *>(
      const_cast<const Column *>(this)->get(entityId));
}

inline const std::any *Column::get(EntityId entityId) const {
  if (auto it = components_.find(entityId); it != components_.end()) {
    return &it->second;
  } else {
    return nullptr;
  }
}

inline std::size_t Column::size() const noexcept { return components_.size(); }

inline decltype(auto) Column::getEntityIds() const {
  return components_ | std::views::keys;
}

template <Component T>
class Components {
public:
  explicit Components(Column& column) : column_(column) {}

  class Iter {
    using UnderlyingIter = std::map<EntityId, std::any>::iterator;

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::pair<EntityId, T&>;
    using pointer = void;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

    Iter() : it_() {}
    explicit Iter(UnderlyingIter it) : it_(it) {}

    value_type operator*() const {
      return {it_->first, std::any_cast<T&>(it_->second)};
    }

    Iter& operator++() {
      ++it_;
      return *this;
    }
    Iter operator++(int) {
      Iter value = *this;
      ++it_;
      return value;
    }

    bool operator<=>(const Iter& other) const = default;

  private:
    UnderlyingIter it_ = UnderlyingIter{};
  };

  [[nodiscard]] Iter begin() noexcept { return Iter(column_.begin()); }
  [[nodiscard]] Iter end() noexcept { return Iter(column_.end()); }
  [[nodiscard]] Iter begin() const noexcept { return Iter(column_.begin()); }
  [[nodiscard]] Iter end() const noexcept { return Iter(column_.end()); }

private:
  Column& column_;
};
static_assert(std::forward_iterator<Components<int>::Iter>);

}  // namespace bad
