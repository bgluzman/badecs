#ifndef BADECS_VIEW_H
#define BADECS_VIEW_H

#include <badecs/Common.h>
#include <badecs/internal/FilterList.h>
#include <badecs/internal/ViewImpl.h>

namespace bad {

/// \brief A set of components to filter from a view.
// TODO (bgluzman): should this be relocated?
template <Component... Ts>
inline constexpr internal::FilterList<Ts...> filter{};

/// \brief A view into a set of components.
/// \tparam Ts The types of components to view.
template <Component... Ts>
class View {

public:
  /// \brief [FOR INTERNAL USAGE] Constructs a view.
  explicit View(internal::ViewImpl<Ts...> impl) : impl_(std::move(impl)) {}

  /// \brief Returns an iterator to the beginning of the view.
  /// \return An iterator to the beginning of the view.
  [[nodiscard]] auto begin() noexcept { return impl_.begin(); }

  /// \brief Returns an iterator to the end of the view.
  /// \return An iterator to the end of the view.
  [[nodiscard]] auto end() noexcept { return impl_.end(); }

private:
  internal::ViewImpl<Ts...> impl_;
};

};  // namespace bad

#endif