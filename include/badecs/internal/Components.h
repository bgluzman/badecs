#ifndef BADECS_INTERNAL_COMPONENTS_H
#define BADECS_INTERNAL_COMPONENTS_H

#include <any>
#include <badecs/Common.h>
#include <badecs/internal/Column.h>
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

  // TODO (bgluzman): docstring
  template <Component T>
  void set(EntityId entity, const T& value) {
    components_[componentId<T>].set(entity, value);
  }

  // TODO (bgluzman): docstring
  template <Component T>
  bool remove(EntityId /*entity*/) {
    // TODO (bgluzman)
    return false;
  }

  // TODO (bgluzman): docstring
  template <std::ranges::range T>
  void removeAll(EntityId /*entity*/, const T& /*componentsToRemove*/) {
    // TODO (bgluzman)
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

private:
  std::unordered_map<ComponentId, Column> components_;
};

}  // namespace bad::internal

#endif