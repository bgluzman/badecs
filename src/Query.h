#pragma once

#include "Commands.h"
#include "World.h"

#include <gsl/gsl>

namespace bad {

struct QueryTags {
  struct EntityId {};
  struct Commands {};
};

template <Component... Args>
class Query {
  template <Component...>
  friend class Query;
  friend Query<> query(gsl::not_null<World *>);

public:
  template <Component Arg, ArgOrder order = ArgOrder::Append>
  auto with() {
    std::set<EntityId> result;
    std::ranges::set_intersection(entities_,
                                  world_->entitiesWithComponent<Arg>(),
                                  std::inserter(result, result.begin()));
    if constexpr (order == ArgOrder::Prepend) {
      return Query<Arg, Args...>(world_, std::move(result));
    } else if constexpr (order == ArgOrder::Append) {
      return Query<Args..., Arg>(world_, std::move(result));
    } else {
      static_assert(always_false_v<ArgOrder>, "Invalid ArgOrder");
    }
  }

  template <Component Arg>
  Query<Args...> filter() {
    std::set<EntityId> result;
    std::ranges::set_difference(entities_, world_->entitiesWithComponent<Arg>(),
                                std::inserter(result, result.begin()));
    return Query<Args...>(world_, std::move(result));
  }

  void each(EachFunctor<Args...> auto&& functor) {
    for (EntityId id : entities_) {
      functor(*world_->getComponent<Args>(id)...);
    }
  }
  void each(QueryTags::EntityId, EachFunctor<Args...> auto&& functor) {
    for (EntityId id : entities_) {
      functor(id, *world_->getComponent<Args>(id)...);
    }
  }
  void each(QueryTags::Commands, EachFunctor<Args...> auto&& functor) {
    Commands commands;
    for (EntityId id : entities_) {
      functor(commands, *world_->getComponent<Args>(id)...);
    }
    commands.execute(world_);
  }
  void each(QueryTags::EntityId, QueryTags::Commands,
            EachFunctor<Args...> auto&& functor) {
    Commands commands;
    for (EntityId id : entities_) {
      functor(id, commands, *world_->getComponent<Args>(id)...);
    }
    commands.execute(world_);
  }

private:
  // Note this could be made more efficient with a partial specialization where
  //   template <Component Arg>
  //   class Query<Arg> {...};
  // only has a constructor which initializes its entities_ to
  // world_->entitiesWithComponent<Arg>() and ONLY has the with() and each()
  // member functions. However, that complicates the implementation quite a bit,
  // so we stick with this approach for now.
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

template <Component Arg, Component... Args>
Query<Arg, Args...> query(gsl::not_null<World *> world) {
  if constexpr (sizeof...(Args) == 0) {
    return query(world).with<Arg>();
  } else {
    return query<Args...>(world).template with<Arg, ArgOrder::Prepend>();
  }
}

}  // namespace bad