#ifndef BADECS_INTERNAL_COLUMN_H
#define BADECS_INTERNAL_COLUMN_H

#include <badecs/Common.h>

#include <any>
#include <map>

namespace bad::internal {

// TODO (bgluzman): docstring
class Column {

public:
  // TODO (bgluzman): docstring
  template <Component T, typename... Ts>
  void emplace(EntityId /*entityId*/, Ts &&.../*args*/) {
    // TODO (bgluzman): implement
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

  // TODO (bgluzman): docstring
  [[nodiscard]] bool has(EntityId /*entityId*/) const noexcept {
    // TODO (bgluzman): implement
    return false;
  }

  // TODO (bgluzman): docstring
  [[nodiscard]] std::any *get(EntityId /*entityId*/) {
    // TODO (bgluzman): implement
  }

  // TODO (bgluzman): docstring
  [[nodiscard]] const std::any *get(EntityId /*entityId*/) const {
    // TODO (bgluzman): implement
  }

  // TODO (bgluzman): docstring
  [[nodiscard]] std::size_t size() const noexcept {
    // TODO (bgluzman): implement
    return 0;
  }

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
