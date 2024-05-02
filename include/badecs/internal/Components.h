#ifndef BADECS_INTERNAL_COMPONENTS_H
#define BADECS_INTERNAL_COMPONENTS_H

#include <any>
#include <badecs/Common.h>
#include <badecs/internal/Column.h>
#include <badecs/internal/FilterList.h>
#include <badecs/internal/ViewImpl.h>
#include <ranges>
#include <unordered_map>

namespace bad::internal {

/// \brief Storage of components in our registry.
///
/// This class stores components together in a `Column` which are indexed by
/// their `ComponentId`.
class Components {
public:
  /// Emplace-constructs a component of type T for the given entity.
  /// \tparam T The type of the component to construct.
  /// \tparam Ts The types of the arguments to pass to the constructor of T.
  /// \param entity The entity to associate with the component.
  /// \param args The arguments to pass to the constructor of T.
  template <Component T, typename... Ts>
  void emplace(EntityId entity, Ts&&...args) {
    components_[componentId<T>].template emplace<T>(entity,
                                                    std::forward<Ts>(args)...);
  }

  /// Sets the component of type T for the given entity to the given value.
  /// \tparam T The type of the component to set.
  /// \param entity The entity to associate with the component.
  /// \param value The new value for the component.
  template <Component T>
  void set(EntityId entity, const T& value) {
    components_[componentId<T>].set(entity, value);
  }

  /// Removes the component of type T for the given entity.
  /// \tparam T The type of the component to remove.
  /// \param entity The entity from which to remove the component.
  /// \return True if the component was removed, false if no such component.
  template <Component T>
  bool remove(EntityId entity) {
    if (auto it = components_.find(componentId<T>); it != components_.end()) {
      return it->second.remove(entity);
    }
    return false;
  }

  template <std::ranges::range Range>
    requires(
        std::is_convertible_v<std::ranges::range_value_t<Range>, ComponentId>)
  void remove(EntityId entity, const Range& components) {
    for (ComponentId component : components) {
      if (auto it = components_.find(component); it != components_.end()) {
        it->second.remove(entity);
      }
    }
  }

  /// Returns true if a component of type T exists for the given entity.
  /// \tparam T The type of the component.
  /// \param entity The entity to check.
  /// \return True if a component of type T exists for the given entity.
  template <typename T>
  bool has(EntityId entity) const noexcept {
    if (auto it = components_.find(componentId<T>); it != components_.end()) {
      return it->second.has(entity);
    }
    return false;
  }

  /// Returns a pointer to the component of type T for the given entity, or
  /// nullptr if no such component exists.
  /// \tparam T The type of the component.
  /// \param entity The entity for which we are fetching the component.
  /// \return A pointer to the component of type T, or nullptr if no such
  /// component exists.
  template <Component T>
  T *get(EntityId entity) {
    return const_cast<T *>(
        const_cast<const Components *>(this)->get<T>(entity));
  }

  /// Returns a pointer to the component of type T for the given entity, or
  /// nullptr if no such component exists.
  /// \tparam T The type of the component.
  /// \param entity The entity for which we are fetching the component.
  /// \return A pointer to the component of type T, or nullptr if no such
  /// component exists.
  template <Component T>
  const T *get(EntityId entity) const {
    if (auto it = components_.find(componentId<T>); it != components_.end()) {
      return std::any_cast<T>(it->second.get(entity));
    }
    return nullptr;
  }

  /// Returns an iterable view over the specified component types.
  /// \tparam Ts The types of components to view.
  /// \tparam Filters The filters to apply to the view.
  /// \return An iterable view object.
  template <Component... Ts, FilterListLike Filters = FilterList<>>
  ViewImpl<Ts...> view(Filters = {}) {
    ViewImpl<Ts...> view{{GetColumn<Ts>()...}};
    addFilters<Filters>(view);
    return view;
  }

  /// Returns an iterable view over the specified component types.
  /// \tparam Ts The types of components to view.
  /// \tparam Filters The filters to apply to the view.
  /// \return An iterable view object.
  template <Component... Ts, FilterListLike Filters = FilterList<>>
  ViewImpl<const Ts...> view(Filters = {}) const {
    // const-cast here is fine since the returned view is unable to modify the
    // contents of these columns.
    ViewImpl<const Ts...> view{{const_cast<Column *>(GetColumn<Ts>())...}};
    addFilters<Filters>(view);
    return view;
  }

private:
  template <Component T>
  Column *GetColumn() {
    return const_cast<Column *>(
        const_cast<const Components *>(this)->GetColumn<T>());
  }

  template <Component T>
  const Column *GetColumn() const {
    if (auto it = components_.find(componentId<T>); it != components_.end()) {
      return &it->second;
    }
    return nullptr;
  }

  template <FilterListLike Filters, typename View>
  void addFilters(View& view) const {
    using Head = typename Filters::Head;
    using Tail = typename Filters::Tail;
    if constexpr (!std::is_same_v<Head, void>) {
      if (auto it = components_.find(componentId<typename Filters::Head>);
          it != components_.end()) {
        view.filterColumn(gsl::not_null(&it->second));
      }
    }
    if constexpr (!std::is_same_v<Tail, void>) {
      addFilters<Tail>(view);
    }
  }

  std::unordered_map<ComponentId, Column> components_;
};

}  // namespace bad::internal

#endif