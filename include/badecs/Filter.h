#ifndef BADECS_FILTER_H
#define BADECS_FILTER_H

#include <badecs/Common.h>
#include <badecs/internal/FilterList.h>

namespace bad {

/// \brief A set of components to filter from a view.
template <Component... Ts>
inline constexpr internal::FilterList<Ts...> filter{};

}  // namespace bad

#endif
