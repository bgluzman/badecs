#pragma once

#include "Common.h"
#include "World.h"

#include <functional>
#include <gsl/gsl>
#include <vector>

namespace bad {

class Commands {

public:
  explicit Commands(gsl::not_null<World *> world);

  EntityId create();
  void     destroy(EntityId entity);

  template <Component T, typename... Ts>
  void emplaceComponent(EntityId entity, Ts&&...args);
  template <Component T>
  void setComponent(EntityId entity, const T& value);
  template <Component T>
  void removeComponent(EntityId entity);

  void execute();

private:
  using Command = std::function<void(World *)>;

  gsl::not_null<World *> world_;
  std::vector<Command>   commands_;
};

inline Commands::Commands(gsl::not_null<World *> world) : world_(world) {}

inline EntityId Commands::create() {
  EntityId entity = world_->reserve();
  commands_.emplace_back(
      [entity](World *world) { world->instantiate(entity); });
  return entity;
}

inline void Commands::destroy(EntityId entity) {
  commands_.emplace_back([entity](World *world) { world->destroy(entity); });
}

template <Component T, typename... Ts>
void Commands::emplaceComponent(EntityId entity, Ts&&...args) {
  commands_.emplace_back(
      [entity, ... args = std::forward<Ts>(args)](World *world) mutable {
        world->emplaceComponent<T>(entity, std::forward<Ts>(args)...);
      });
}

template <Component T>
void Commands::setComponent(EntityId entity, const T& value) {
  commands_.emplace_back([entity, value](World *world) mutable {
    world->setComponent(entity, value);
  });
}

template <Component T>
void Commands::removeComponent(EntityId entity) {
  commands_.emplace_back(
      [entity](World *world) { world->removeComponent<T>(entity); });
}

inline void Commands::execute() {
  for (const Command& command : commands_) {
    command(world_.get());
  }
  commands_.clear();
}

}  // namespace bad