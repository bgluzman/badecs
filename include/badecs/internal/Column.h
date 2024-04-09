#ifndef BADECS_INTERNAL_COLUMN_H
#define BADECS_INTERNAL_COLUMN_H

#include <any>
#include <badecs/Common.h>
#include <unordered_map>

namespace bad::internal {

/// \brief Entity-indexed storage of components sharing the same type.
///
/// This class erases the type of the components stored in it. The user is then
/// responsible for extracting the components into their original type. This
/// allows us to treat a collection of columns of different types uniformly.
class Column {

public:
  /// Emplace-constructs a component of type T for the given entity.
  /// \tparam T The type of the component to construct.
  /// \tparam Args The types of the arguments to pass to the constructor of T.
  /// \param entity The entity to associate with the component.
  /// \param args The arguments to pass to the constructor of T.
  template <Component T, typename... Args>
  void emplace(EntityId entity, Args&&...args) {
    components_[entity] =
        std::any(std::in_place_type<T>, std::forward<Args>(args)...);
  }

  /// Sets the component for the given entity to the given value.
  /// \param entity The entity to associate with the component.
  /// \param value The new value for the component.
  void set(EntityId entity, std::any value) {
    components_[entity] = std::move(value);
  }

  /// Removes the component for the given entity.
  /// \param entity The entity from which to remove the component.
  /// \return True if the component was removed, false if no such component.
  bool remove(EntityId entity) { return components_.erase(entity) > 0; }

  /// Returns true if a component for the given entity exists in this column.
  /// \param entity The entity to check.
  /// \return True if a component exists for the given entity.
  [[nodiscard]] bool has(EntityId entity) const noexcept {
    return components_.count(entity) > 0;
  }

  /// Returns a pointer to the type-erased component for the given entity, or
  /// nullptr if no such component exists.
  /// \param entity The entity for which we are fetching the component.
  /// \return A pointer to a std::any storing the component, or nullptr if no
  ///  such component exists.
  [[nodiscard]] std::any *get(EntityId entity) {
    return const_cast<std::any *>(
        const_cast<const Column *>(this)->get(entity));
  }

  /// Returns a pointer to the type-erased component for the given entity, or
  /// nullptr if no such component exists.
  /// \param entity The entity for which we are fetching the component.
  /// \return A pointer to a std::any storing the component, or nullptr if no
  ///  such component exists.
  [[nodiscard]] const std::any *get(EntityId entity) const {
    if (auto it = components_.find(entity); it != components_.end()) {
      return &it->second;
    } else {
      return nullptr;
    }
  }

  /// Returns the number of components stored in this column.
  /// \return The number of components stored in this column.
  [[nodiscard]] std::size_t size() const noexcept { return components_.size(); }

  /// Returns an iterator to the beginning of the components in this column.
  /// \return An iterator to the beginning of the components in this column.
  [[nodiscard]] auto begin() noexcept { return components_.begin(); }

  /// Returns an iterator to the end of the components in this column.
  /// \return An iterator to the end of the components in this column.
  [[nodiscard]] auto end() noexcept { return components_.end(); }

private:
  // XXX: std::map's pointer/reference invalidation semantics here are
  //  incredibly important since get() returns a pointer into the map. If
  //  another map implementation is chosen, it is possible that get() should
  //  have different semantics OR the values in the map should be pointers.
  std::unordered_map<EntityId, std::any> components_ = {};
};

}  // namespace bad::internal

#endif  // BADECS_INTERNAL_COLUMN_H
