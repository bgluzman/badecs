#pragma once

#include <concepts>
#include <cstdint>

namespace bad {

class EntityHandle;  // forward declaration for concept defs

// CopyConstructible is a requirement for std::any
template <typename T>
concept Component = std::is_copy_constructible_v<T>;

template <typename F, typename... Args>
concept QueryFunctor =
    std::invocable<F, Args...> || std::invocable<F, EntityHandle, Args...>;

using EntityId = std::uint32_t;
using ComponentId = std::uint32_t;

}  // namespace bad
