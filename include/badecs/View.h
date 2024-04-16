#ifndef BADECS_VIEW_H
#define BADECS_VIEW_H

#include <any>
#include <array>
#include <badecs/Common.h>
#include <badecs/internal/Column.h>
#include <badecs/internal/FilterList.h>
#include <gsl/gsl>
#include <unordered_map>
#include <vector>

namespace bad {

// TODO (bgluzman): docstring
template <Component... Ts>
inline constexpr internal::FilterList<Ts...> filter{};

// TODO (bgluzman): docstring
template <Component... Ts>
class View {
  using ColumnArray = std::array<internal::Column *, sizeof...(Ts)>;
  static_assert(sizeof...(Ts) > 0, "View must have at least one component");

public:
  // TODO (bgluzman): docstring
  explicit View(std::array<internal::Column *, sizeof...(Ts)> columns);

  // TODO (bgluzman): docstring
  void addFilter(gsl::not_null<internal::Column *> column);

  // TODO (bgluzman): docstring
  class Iterator {
    using UnderlyingIter = std::unordered_map<EntityId, std::any>::iterator;

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::tuple<EntityId, Ts...>;
    using pointer = void;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

    // TODO (bgluzman): docstring
    Iterator();

    // TODO (bgluzman): docstring
    Iterator(ColumnArray                                    cols,
             std::vector<gsl::not_null<internal::Column *>> filters,
             UnderlyingIter it, UnderlyingIter end);

    // TODO (bgluzman): docstring
    value_type operator*() const;
    // TODO (bgluzman): docstring
    Iterator& operator++();
    // TODO (bgluzman): docstring
    Iterator operator++(int);

    // TODO (bgluzman): docstring
    bool operator==(const Iterator& other) const;
    // TODO (bgluzman): docstring
    bool operator!=(const Iterator& other) const;

  private:
    // TODO (bgluzman): docstring
    void advance();

    ColumnArray                                    columns_;
    std::vector<gsl::not_null<internal::Column *>> filters_;
    UnderlyingIter                                 it_;
    UnderlyingIter                                 end_;
  };
  // TODO (bgluzman): reenable
  // static_assert(std::forward_iterator<Iterator>);

  // TODO (bgluzman): docstring
  [[nodiscard]] Iterator begin() noexcept;
  // TODO (bgluzman): docstring
  [[nodiscard]] Iterator end() noexcept;

private:
  bool                                           isEmptyMarker_;
  ColumnArray                                    columns_;
  typename ColumnArray::size_type                minIdx_;
  std::vector<gsl::not_null<internal::Column *>> filters_ = {};
};

}  // namespace bad

#endif