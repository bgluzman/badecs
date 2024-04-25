#ifndef BADECS_VIEW_H
#define BADECS_VIEW_H

#include <any>
#include <array>
#include <badecs/Common.h>
#include <badecs/internal/Column.h>
#include <badecs/internal/FilterList.h>
#include <gsl/gsl>
#include <limits>
#include <unordered_map>
#include <vector>

namespace bad {

/// \brief A set of components to filter from a view.
// TODO (bgluzman): should this be relocated?
template <Component... Ts>
inline constexpr internal::FilterList<Ts...> filter{};

/// \brief A view into a set of components.
/// \tparam Ts The types of components to view.
///
/// Takes in a set of `Column` objects (which do not know the types of their
/// stored components) and provides a typed view over them. The view is accessed
/// through an iterator that returns a tuple of entity-id and the components.
/// To make iteration more efficient, we iterate over the smallest column's
/// underlying iterator and then select the components from the other columns.
template <Component... Ts>
class View {
  using ColumnArray = std::array<internal::Column *, sizeof...(Ts)>;
  static_assert(sizeof...(Ts) > 0, "View must have at least one component");

public:
  /// \brief Constructs a view from a set of columns.
  /// \param columns The columns to view.
  explicit View(std::array<internal::Column *, sizeof...(Ts)> columns)
      : isEmpty_(false), columns_(columns),
        minIdx_(std::numeric_limits<std::size_t>::max()) {
    // Check if any column is null or empty and cache the result in `isEmpty_`.
    // That way, we do not need to recompute it every time we iterate over the
    // view.
    if (std::any_of(columns_.begin(), columns_.end(),
                    [](auto col) { return !col || col->size() == 0; })) {
      isEmpty_ = true;
    } else {
      auto minElem = std::min_element(
          columns_.begin(), columns_.end(),
          [](auto lhs, auto rhs) { return lhs->size() < rhs->size(); });
      minIdx_ = std::distance(columns_.begin(), minElem);
    }
  }

  /// \brief Add column whose entity-ids will be filtered from the view.
  /// \param column The column to filter.
  void filterColumn(gsl::not_null<internal::Column *> column) {
    filters_.push_back(column);
  }

  /// \brief An iterator over the components in the view.
  ///
  /// Iterates over the components in the view, filtering out any entity-ids
  /// present in the filter columns. Iteration is done transparently over the
  /// underlying iterators of the view columns.
  class Iterator {
    using UnderlyingIter = std::unordered_map<EntityId, std::any>::iterator;

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::tuple<EntityId, Ts...>;
    using pointer = void;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

    /// \brief Default constructor.
    Iterator() : columns_(), filters_(), it_(), end_() {}

    /// \brief Constructs an iterator.
    /// \param cols The columns to iterate over.
    /// \param filters The columns holding the entity-ids to filter.
    /// \param begin The starting position within the underlying iterator.
    /// \param end The end position with the underlying iterator.
    Iterator(ColumnArray                                           cols,
             const std::vector<gsl::not_null<internal::Column *>>& filters,
             UnderlyingIter begin, UnderlyingIter end)
        : columns_(cols), filters_(filters), it_(begin), end_(end) {
      advance();  // Start at the first position that is valid and not filtered.
    }

    /// \brief Returns a tuple of <EntityId, Ts...> for the current set of
    /// components in the view.
    /// \return A tuple containing the entity-id and the components.
    value_type operator*() const {
      auto helper = [this]<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::tuple<EntityId, Ts&...>{
            it_->first, *std::any_cast<std::remove_cvref_t<Ts>>(
                            columns_[Is]->get(it_->first))...};
      };
      return helper(std::index_sequence_for<Ts...>{});
    }

    /// \brief Advances the iterator to the next set of components in the view.
    /// \return A reference to the iterator.
    Iterator& operator++() {
      ++it_;
      advance();
      return *this;
    }

    /// \brief Advances the iterator to the next set of components in the view.
    /// \return A copy of the iterator before the advance.
    Iterator operator++(int) {
      Iterator value = *this;
      ++it_;
      advance();
      return value;
    }

    /// \brief Compares two iterators for equality.
    /// \param other The iterator to compare against.
    /// \return `true` if the iterators are equal, `false` otherwise.
    bool operator==(const Iterator& other) const { return it_ == other.it_; }

    /// \brief Compares two iterators for inequality.
    /// \param other The iterator to compare against.
    /// \return `true` if the iterators are not equal, `false` otherwise.
    bool operator!=(const Iterator& other) const { return it_ != other.it_; }

  private:
    /// \brief Advances the iterator to the next set of components in the view
    /// that are not filtered-out.
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

    ColumnArray                                    columns_;
    std::vector<gsl::not_null<internal::Column *>> filters_;
    UnderlyingIter                                 it_;
    UnderlyingIter                                 end_;
  };
  static_assert(std::forward_iterator<Iterator>);

  /// \brief Returns an iterator to the beginning of the view.
  /// \return An iterator to the beginning of the view.
  [[nodiscard]] Iterator begin() noexcept {
    if (isEmpty_) {
      return Iterator();
    }
    return Iterator(columns_, filters_, columns_[minIdx_]->begin(),
                    columns_[minIdx_]->end());
  }

  /// \brief Returns an iterator to the end of the view.
  /// \return An iterator to the end of the view.
  [[nodiscard]] Iterator end() noexcept {
    if (isEmpty_) {
      return Iterator();
    }
    return Iterator(columns_, filters_, columns_[minIdx_]->end(),
                    columns_[minIdx_]->end());
  }

private:
  bool                                           isEmpty_;
  ColumnArray                                    columns_;
  typename ColumnArray::size_type                minIdx_;
  std::vector<gsl::not_null<internal::Column *>> filters_ = {};
};

}  // namespace bad

#endif