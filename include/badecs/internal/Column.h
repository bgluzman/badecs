#ifndef BADECS_INTERNAL_COLUMN_H
#define BADECS_INTERNAL_COLUMN_H

#include <badecs/Common.h>

#include <any>
#include <map>

namespace bad::internal {

/// \brief Entity-indexed storage of components sharing the same type.
///
/// This class erases the type of the components stored in it. The user is then
/// responsible for extracting the components into their original type. This
/// allows us to treat a collection of columns of different types uniformly.
class Column {

public:
  /// Emplace-constructs a component of type T for the given entity.
  template <Component T, typename... Args>
  void emplace(EntityId entityId, Args &&...args) {
    components_[entityId] =
        std::any(std::in_place_type<T>, std::forward<Args>(args)...);
  }

  // TODO (bgluzman): docstring
  void set(EntityId /*entityId*/, std::any /*value*/) {
    // TODO (bgluzman): implement
  }

  // TODO (bgluzman): docstring
  bool remove(EntityId /*entityId*/) {
    // TODO (bgluzman): implement
    return false;
  }

  /// Returns true if a component for the given entity exists in this column.
  [[nodiscard]] bool has(EntityId entityId) const noexcept {
    return components_.count(entityId) > 0;
  }

  // TODO (bgluzman): docstring
  [[nodiscard]] std::any *get(EntityId /*entityId*/) {
    // TODO (bgluzman): implement
  }

  // TODO (bgluzman): docstring
  [[nodiscard]] const std::any *get(EntityId /*entityId*/) const {
    // TODO (bgluzman): implement
  }

  /// Returns the number of components stored in this column.
  [[nodiscard]] std::size_t size() const noexcept { return components_.size(); }

  // TODO (bgluzman): docstring
  [[nodiscard]] auto begin() noexcept { return components_.begin(); }

  // TODO (bgluzman): docstring
  [[nodiscard]] auto end() noexcept { return components_.end(); }

private:
  // XXX: std::map's pointer/reference invalidation semantics here are
  //  incredibly important since get() returns a pointer into the map. If
  //  another map implementation is chosen, it is possible that get() should
  //  have different semantics OR the values in the map should be pointers.
  std::map<EntityId, std::any> components_ = {};
};

} // namespace bad::internal

#endif // BADECS_INTERNAL_COLUMN_H
