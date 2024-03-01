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
template <typename T>
concept MetaArg = std::is_same_v<T, EntityId> || std::is_same_v<T, Commands> ||
                  std::is_same_v<T, void>;
template <typename F, typename Arg0, typename Arg1, typename... Args>
concept EachFunctor =
    MetaArg<Arg0> && MetaArg<Arg1> &&
    (std::is_same_v<Arg0, void> && std::invocable<F, Args&...> ||
     std::is_same_v<Arg1, void> && std::invocable<F, Arg0&, Args&...> ||
     std::invocable<F, Arg0&, Arg1&, Args&...>);

enum class ArgOrder { Prepend, Append };

template <typename>
inline constexpr bool always_false_v = std::false_type::value;

}  // namespace bad
