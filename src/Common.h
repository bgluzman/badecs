#pragma once

#include <concepts>
#include <cstdint>

namespace bad {

// CopyConstructible is a requirement for std::any
template <typename T>
concept Component = std::is_copy_constructible_v<T>;

using EntityId = std::uint32_t;
using ComponentId = std::uint32_t;

}  // namespace bad
