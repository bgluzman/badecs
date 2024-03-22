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

template <Component T>
class Components {
public:
  explicit Components(Column& column) : column_(column) {}

  template <typename... Ts>
  void emplace(EntityId entityId, Ts&&...args) {
    return column_.emplace<T>(entityId, std::forward<Ts>(args)...);
  }
  void set(EntityId entityId, const T& value) {
    return column_.set(entityId, value);
  }
  bool remove(EntityId entityId) { return column_.remove(entityId); }
  [[nodiscard]] bool has(EntityId entityId) const noexcept {
    return column_.has(entityId);
  }
  [[nodiscard]] std::any *get(EntityId entityId) {
    return column_.get(entityId);
  }
  [[nodiscard]] const std::any *get(EntityId entityId) const {
    return column_.get(entityId);
  }

  [[nodiscard]] std::size_t size() const noexcept { return column_.size(); }

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

template <Component... Ts>
class View {
  static_assert(sizeof...(Ts) > 0, "View must have at least one component");

public:
  explicit View(std::array<gsl::not_null<Column *>, sizeof...(Ts)> columns)
      : columns_(columns), minIdx_(0) {
    auto minElem = std::min_element(
        columns_.begin(), columns_.end(),
        [](auto lhs, auto rhs) { return lhs->size() < rhs->size(); });
    minIdx_ = std::distance(columns_.begin(), minElem);
  }

  class Iterator {
    using UnderlyingIter = std::map<EntityId, std::any>::iterator;

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::tuple<EntityId, Ts...>;
    using pointer = void;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

    Iterator() : columns_(), it_() {}
    explicit Iterator(std::array<gsl::not_null<Column *>, sizeof...(Ts)> cols,
                      UnderlyingIter it, UnderlyingIter end)
        : columns_(cols), it_(it), end_(end) {
      advance();
    }

    value_type operator*() const {
      auto helper = [this]<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::tuple<EntityId, Ts&...>{
            it_->first, *std::any_cast<std::remove_cvref_t<Ts>>(
                            columns_[Is]->get(it_->first))...};
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
      for (; it_ != end_; ++it_) {
        EntityId id = it_->first;
        if (std::all_of(columns_.begin(), columns_.end(),
                        [id](auto col) { return col->has(id); })) {
          return;
        }
      }
    }

    std::array<gsl::not_null<Column *>, sizeof...(Ts)> columns_;
    UnderlyingIter                                     it_;
    UnderlyingIter                                     end_;
  };
  static_assert(std::forward_iterator<Iterator>);

  [[nodiscard]] Iterator begin() noexcept {
    return Iterator(columns_, columns_[minIdx_]->begin(),
                    columns_[minIdx_]->end());
  }
  [[nodiscard]] Iterator end() noexcept {
    return Iterator(columns_, columns_[minIdx_]->end(),
                    columns_[minIdx_]->end());
  }

private:
  std::array<gsl::not_null<Column *>, sizeof...(Ts)>            columns_;
  std::array<gsl::not_null<Column *>, sizeof...(Ts)>::size_type minIdx_;
};

}  // namespace bad
