#pragma once

#include "World.h"

#include <gsl/gsl>

namespace bad {

template <Component... Args>
class QueryBuilder {

public:
  explicit QueryBuilder(gsl::not_null<World *> world,
                        std::set<EntityId>     entities = {})
      : world_(world), entities_(std::move(entities)) {
    if (sizeof...(Args) == 0) {
      // When initially constructed, we query everything by default.
      entities_ = world_->entities();
    }
  }

  template <Component Arg>
  QueryBuilder<Arg, Args...> With();
  template <Component Arg>
  QueryBuilder<Args...> Without();

  void each(ForEachFunctor<Args...> auto&& functor) const {
    for (EntityId id : entities_) {
      if constexpr (ForEachSimple<decltype(functor), Args...>) {
        functor(*world_->getComponent<Args>(id)...);
      } else if constexpr (ForEachWithEntityId<decltype(functor), Args...>) {
        functor(id, *world_->getComponent<Args>(id)...);
      } else {
        static_assert(always_false_v<decltype(functor)>,
                      "Invalid functor signature");
      }
    }
  }

private:
  std::set<EntityId>     entities_ = {};
  gsl::not_null<World *> world_;
};

template <Component... Args>
template <Component Arg>
QueryBuilder<Arg, Args...> QueryBuilder<Args...>::With() {
  std::set<EntityId> result;
  std::ranges::set_intersection(entities_, world_->entitiesWithComponent<Arg>(),
                                std::inserter(result, result.begin()));
  return QueryBuilder<Arg, Args...>(world_, std::move(result));
}

template <Component... Args>
template <Component Arg>
QueryBuilder<Args...> QueryBuilder<Args...>::Without() {
  std::set<EntityId> result;
  std::ranges::set_difference(entities_, world_->entitiesWithComponent<Arg>(),
                              std::inserter(result, result.begin()));
  return QueryBuilder<Args...>(world_, std::move(result));
}

}  // namespace bad