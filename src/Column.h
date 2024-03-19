#pragma once

#include "Common.h"

#include <any>
#include <array>
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

template <bool DerefAsValue, Component... Ts>
class View {
  static_assert(sizeof...(Ts) > 0, "View must have at least one component");

public:
  explicit View(std::array<gsl::not_null<Column *>, sizeof...(Ts)> columns)
      : columns_(columns) {
    std::sort(columns_.begin(), columns_.end(),
              [](auto lhs, auto rhs) { return lhs->size() < rhs->size(); });
  }

  class Iterator {
    using UnderlyingIter = std::map<EntityId, std::any>::iterator;

  public:
    using difference_type = std::ptrdiff_t;
    using value_type =
        std::conditional_t<DerefAsValue, std::tuple<EntityId, Ts...>,
                           std::tuple<EntityId, Ts&...>>;
    using pointer = void;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

    Iterator() : cols_(), it_() {}
    explicit Iterator(std::array<gsl::not_null<Column *>, sizeof...(Ts)> cols,
                      UnderlyingIter                                     it)
        : cols_(cols), it_(it) {
      advance();
    }

    value_type operator*() const {
      auto helper = [this]<std::size_t... Is>(std::index_sequence<Is...>) {
        if constexpr (DerefAsValue) {
          return std::tuple<EntityId, Ts...>{
              it_->first, std::any_cast<Ts>(*cols_[Is]->get(it_->first))...};
        } else {
          return std::tuple<EntityId, Ts&...>{
              it_->first, *std::any_cast<Ts>(cols_[Is]->get(it_->first))...};
        }
      };
      return helper(std::index_sequence_for<Ts...>{});
    }

    Iterator& operator++() {
      ++it_;
      advance();
      return *this;
    }
    Iterator operator++(int) {
      Iterator value = *this;
      ++it_;
      advance();
      return value;
    }

    bool operator==(const Iterator& other) const { return it_ == other.it_; }
    bool operator!=(const Iterator& other) const { return it_ != other.it_; }

  private:
    void advance() {
      for (; it_ != cols_[0]->end(); ++it_) {
        EntityId id = it_->first;
        if (std::all_of(cols_.begin(), cols_.end(),
                        [id](auto col) { return col->has(id); })) {
          return;
        }
      }
    }

    std::array<gsl::not_null<Column *>, sizeof...(Ts)> cols_;
    UnderlyingIter                                     it_;
  };
  static_assert(std::forward_iterator<Iterator>);

  [[nodiscard]] Iterator begin() noexcept {
    return Iterator(columns_, columns_[0]->begin());
  }
  [[nodiscard]] Iterator end() noexcept {
    return Iterator(columns_, columns_[0]->end());
  }

private:
  std::array<gsl::not_null<Column *>, sizeof...(Ts)> columns_;
};
static_assert(std::forward_iterator<View<true, int>::Iterator>);
static_assert(std::forward_iterator<View<false, int>::Iterator>);

template <Component... Ts>
View<true, Ts...>
valueView(std::array<gsl::not_null<Column *>, sizeof...(Ts)> cols) {
  return View<true, Ts...>(std::move(cols));
}

template <Component... Ts>
View<false, Ts...>
refView(std::array<gsl::not_null<Column *>, sizeof...(Ts)> cols) {
  return View<false, Ts...>(std::move(cols));
}

}  // namespace bad
