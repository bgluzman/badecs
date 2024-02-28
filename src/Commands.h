#pragma once

#include "Common.h"
#include "World.h"

#include <functional>
#include <gsl/gsl>
#include <vector>

namespace bad {

class Commands {

public:
  //  void spawnEntity();
  //  void destroyEntity();
  //  void removeComponent();
  template <Component T>
  void setComponent(EntityId entity, const T& value);

  void execute(gsl::not_null<World *> world);

private:
  using Command = std::function<void(World *)>;
  std::vector<Command> commands_;
};

template <Component T>
void Commands::setComponent(EntityId entity, const T& value) {
  commands_.push_back([entity, value](World *world) mutable {
    world->setComponent(entity, value);
  });
}

inline void Commands::execute(gsl::not_null<World *> world) {
  for (const Command& command : commands_) {
    command(world.get());
  }
  commands_.clear();
}

}  // namespace bad