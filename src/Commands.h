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
  ~Commands() noexcept;

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

inline Commands::~Commands() noexcept {
  try {
    execute();
  } catch (const std::exception& err) {
    std::cerr << "error destroying Commands: " << err.what() << std::endl;
  } catch (...) {
    std::cerr << "unknown error destroying Commands" << std::endl;
  }
}

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
  std::ptrdiff_t i = 0;
  try {
    for (; i != commands_.size(); ++i) {
      commands_[i](world_.get());
    }
  } catch (...) {
    // Ensures that repeated calls do not execute the same commands.
    // Add +1 here to ensure the command that threw is not executed again.
    commands_.erase(commands_.begin(), commands_.begin() + i + 1);
    throw;
  }
  commands_.clear();
}

class ScopedCommands : public Commands {
public:
  explicit ScopedCommands(gsl::not_null<World *> world) : Commands(world) {}
  ~ScopedCommands() noexcept { execute(); }
};

}  // namespace bad