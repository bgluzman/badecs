#pragma once

#include "World.h"

#include <gsl/gsl>

namespace bad {

template <Component... Args>
class Query {
  template <Component...>
  friend class Query;
  friend Query<> query(gsl::not_null<World *>);

public:
  template <Component Arg>
  Query<Args..., Arg> With() {
    std::set<EntityId> result;
    std::ranges::set_intersection(entities_,
                                  world_->entitiesWithComponent<Arg>(),
                                  std::inserter(result, result.begin()));
    return Query<Args..., Arg>(world_, std::move(result));
  }

  template <Component Arg>
  Query<Args...> Without() {
    std::set<EntityId> result;
    std::ranges::set_difference(entities_, world_->entitiesWithComponent<Arg>(),
                                std::inserter(result, result.begin()));
    return Query<Args...>(world_, std::move(result));
  }

  void each(ForEachFunctor<Args...> auto&& functor) {
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
  explicit Query(gsl::not_null<World *> world, std::set<EntityId> entities = {})
      : world_(world), entities_(std::move(entities)) {
    if (sizeof...(Args) == 0) {
      // When initially constructed, we query everything by default.
      entities_ = world_->allEntities() | std::ranges::to<std::set<EntityId>>();
    }
  }

  std::set<EntityId>     entities_ = {};
  gsl::not_null<World *> world_;
};

inline Query<> query(gsl::not_null<World *> world) { return Query<>(world); }

}  // namespace bad