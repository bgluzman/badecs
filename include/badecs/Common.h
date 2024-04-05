#ifndef BADECS_COMMON_H
#define BADECS_COMMON_H

#include <cstdint>
#include <type_traits>

namespace bad {

using EntityId = std::uint32_t;
using ComponentId = std::uint32_t;

// CopyConstructible is a requirement for std::any
template <typename T>
concept Component = std::is_copy_constructible_v<T>;

} // namespace bad

#endif // BADECS_COMMON_H
