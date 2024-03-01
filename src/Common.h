#pragma once

#include <concepts>
#include <cstdint>

namespace bad {

using EntityId = std::uint32_t;
using ComponentId = std::uint32_t;

// CopyConstructible is a requirement for std::any
template <typename T>
concept Component = std::is_copy_constructible_v<T>;

template <typename F, typename... Args>
concept EachFunctor = std::invocable<F, Args&...>;

enum class ArgOrder { Prepend, Append };

template <typename>
inline constexpr bool always_false_v = std::false_type::value;

}  // namespace bad
