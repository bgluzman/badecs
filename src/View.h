#pragma once

#include "Column.h"

namespace bad {

template <Component... Ts>
struct Filter {};

template <Component T, Component... Ts>
struct Filter<T, Ts...> {
  using Head = T;
  using Tail = Filter<Ts...>;
};

template <Component T>
struct Filter<T> {
  using Head = T;
  using Tail = void;
};

template <>
struct Filter<> {
  using Head = void;
  using Tail = void;
};

template <Component... Ts>
inline constexpr Filter<Ts...> filter{};

template <Component... Ts>
class View {
  static_assert(sizeof...(Ts) > 0, "View must have at least one component");

public:
  explicit View(std::array<Column *, sizeof...(Ts)> columns)
      : isEmptyMarker_(false), columns_(columns),
        minIdx_(std::numeric_limits<std::size_t>::max()) {
    if (std::any_of(columns_.begin(), columns_.end(),
                    [](auto col) { return !col || col->size() == 0; })) {
      isEmptyMarker_ = true;
    } else {
      std::copy(columns.begin(), columns.end(), columns_.begin());
      auto minElem = std::min_element(
          columns_.begin(), columns_.end(),
          [](auto lhs, auto rhs) { return lhs->size() < rhs->size(); });
      minIdx_ = std::distance(columns_.begin(), minElem);
    }
  }

  void addFilter(gsl::not_null<Column *> column) { filters_.push_back(column); }

  class Iterator {
    using UnderlyingIter = std::map<EntityId, std::any>::iterator;

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::tuple<EntityId, Ts...>;
    using pointer = void;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

    Iterator() : columns_(), filters_(), it_(), end_() {}
    Iterator(std::array<Column *, sizeof...(Ts)>  cols,
             std::vector<gsl::not_null<Column *>> filters, UnderlyingIter it,
             UnderlyingIter end)
        : columns_(cols), filters_(std::move(filters)), it_(it), end_(end) {
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
                        [id](auto col) { return col->has(id); }) &&
            std::none_of(filters_.begin(), filters_.end(),
                         [id](auto col) { return col->has(id); })) {
          return;
        }
      }
    }

    std::array<Column *, sizeof...(Ts)>  columns_;
    std::vector<gsl::not_null<Column *>> filters_;
    UnderlyingIter                       it_;
    UnderlyingIter                       end_;
  };
  static_assert(std::forward_iterator<Iterator>);

  [[nodiscard]] Iterator begin() noexcept {
    if (isEmptyMarker_) {
      return Iterator();
    }
    return Iterator(columns_, filters_, columns_[minIdx_]->begin(),
                    columns_[minIdx_]->end());
  }
  [[nodiscard]] Iterator end() noexcept {
    if (isEmptyMarker_) {
      return Iterator();
    }
    return Iterator(columns_, filters_, columns_[minIdx_]->end(),
                    columns_[minIdx_]->end());
  }

private:
  bool                                           isEmptyMarker_;
  std::array<Column *, sizeof...(Ts)>            columns_;
  std::array<Column *, sizeof...(Ts)>::size_type minIdx_;
  std::vector<gsl::not_null<Column *>>           filters_ = {};
};

}  // namespace bad