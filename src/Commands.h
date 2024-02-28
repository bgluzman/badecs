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
  void setComponent(Entity entity, const T& value);

  void execute(gsl::not_null<World *> world);

private:
  enum class CommandType {
    kSpawnEntity,
    kDestroyEntity,
    kRemoveComponent,
    kSetComponent,
  };

  struct Command {
    CommandType                  type;
    std::function<void(World *)> deferred;
  };

  std::vector<Command> commands_;
};

template <Component T>
void Commands::setComponent(Entity entity, const T& value) {
  commands_.push_back({
      .type = CommandType::kSetComponent,
      .deferred = [entity, value](World *) mutable { entity.set(value); },
  });
}

inline void Commands::execute(gsl::not_null<World *> world) {
  for (const Command& command : commands_) {
    command.deferred(world.get());
  }
  commands_.clear();
}

}  // namespace bad