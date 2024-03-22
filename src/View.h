#pragma once

#include "Column.h"

namespace bad {

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