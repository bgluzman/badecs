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
concept QueryFunctor =
    std::invocable<F, Args...> || std::invocable<F, EntityId, Args...>;

}  // namespace bad
