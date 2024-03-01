#pragma once

#include <concepts>
#include <cstdint>

namespace bad {

using EntityId = std::uint32_t;
using ComponentId = std::uint32_t;

// CopyConstructible is a requirement for std::any
template <typename T>
concept Component = std::is_copy_constructible_v<T>;

struct Commands;
template <typename F, typename... Args>
concept EachFunctorSimple = std::invocable<F, Args&...>;
template <typename F, typename... Args>
concept EachFunctorEntity = std::invocable<F, EntityId, Args&...>;
template <typename F, typename... Args>
concept EachFunctorCommands = std::invocable<F, Commands&, Args&...>;
template <typename F, typename... Args>
concept EachFunctorEntityCommands =
    std::invocable<F, EntityId, Commands&, Args&...>;
template <typename F, typename... Args>
concept EachFunctorCommandsEntity =
    std::invocable<F, Commands&, EntityId, Args&...>;

enum class ArgOrder { Prepend, Append };

template <typename>
inline constexpr bool always_false_v = std::false_type::value;

}  // namespace bad
