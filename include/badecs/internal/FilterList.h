#ifndef BADECS_INTERNAL_FILTERLIST_H
#define BADECS_INTERNAL_FILTERLIST_H

#include <badecs/Common.h>
#include <tuple>

namespace bad::internal {

// TODO (bgluzman): docstring
template <Component... Ts>
struct FilterList;

template <Component T, Component... Ts>
struct FilterList<T, Ts...> {
  using FilterListTag = void;
  using Head = T;
  using Tail = FilterList<Ts...>;
};

template <Component T>
struct FilterList<T> {
  using FilterListTag = void;
  using Head = T;
  using Tail = void;
};

template <>
struct FilterList<> {
  using FilterListTag = void;
  using Head = void;
  using Tail = void;
};

template <typename T>
concept FilterListLike = requires {
  typename T::FilterListTag;
  typename T::Head;
  typename T::Tail;
};

}  // namespace bad::internal

#endif