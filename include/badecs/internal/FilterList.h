#ifndef BADECS_INTERNAL_FILTERLIST_H
#define BADECS_INTERNAL_FILTERLIST_H

#include <badecs/Common.h>

namespace bad::internal {

// TODO (bgluzman): docstring
template <Component... Ts>
struct FilterList {};

template <Component T, Component... Ts>
struct FilterList<T, Ts...> {
  using Head = T;
  using Tail = FilterList<Ts...>;
};

template <Component T>
struct FilterList<T> {
  using Head = T;
  using Tail = void;
};

template <>
struct FilterList<> {
  using Head = void;
  using Tail = void;
};

}  // namespace bad::internal

#endif