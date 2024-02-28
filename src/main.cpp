#include "Commands.h"
#include "World.h"

#include <cmath>
#include <complex>
#include <iostream>
#include <string>

struct Vec2 {
  int x;
  int y;
};

struct Position {
  Vec2 value;
};
std::ostream& operator<<(std::ostream& os, const Position& pos) {
  return os << "Position{x=" << pos.value.x << ", y=" << pos.value.y << "}";
}

struct Tag {};
std::ostream& operator<<(std::ostream& os, const Tag&) { return os << "Tag{}"; }

// for playing around with queries...
struct Name {
  std::string name;
};
std::ostream& operator<<(std::ostream& os, const Name& name) {
  return os << "Name{" << name.name << "}";
}
struct Velocity {
  Vec2 value;
};
std::ostream& operator<<(std::ostream& os, const Velocity& vel) {
  return os << "Velocity{x=" << vel.value.x << ", y=" << vel.value.y << "}";
}
struct Player {};
std::ostream& operator<<(std::ostream& os, const Player&) {
  return os << "Player{}";
}
struct Ephemeral {};
std::ostream& operator<<(std::ostream& os, const Ephemeral&) {
  return os << "Ephemeral{}";
}
struct MoveIntention {
  Vec2 value;
};
std::ostream& operator<<(std::ostream& os, const MoveIntention& moveIntention) {
  return os << "MoveIntention{x=" << moveIntention.value.x
            << ", y=" << moveIntention.value.y << "}";
}

int main(int /*argc*/, char * /*argv*/[]) {
  std::cout << std::boolalpha;

  bad::World    world;
  bad::EntityId entity = world.create();

  std::cout << "== before ==";
  std::cout << "\nentity:hasInt=" << world.hasComponent<int>(entity);
  std::cout << "\nentity:hasDouble=" << world.hasComponent<double>(entity);
  std::cout << "\nentity:hasPosition=" << world.hasComponent<Position>(entity);
  std::cout << "\nentity:hasTag=" << world.hasComponent<Tag>(entity);

  world.setComponent<int>(entity, 42);
  world.setComponent<double>(entity, NAN);
  world.emplaceComponent<Tag>(entity);

  std::cout << "\n== after modifications (1) ==";
  std::cout << "\nentity:hasInt=" << world.hasComponent<int>(entity);
  std::cout << "\nentity:int=" << *world.getComponent<int>(entity);
  std::cout << "\nentity:hasDouble=" << world.hasComponent<double>(entity);
  std::cout << "\nentity:double=" << *world.getComponent<double>(entity);
  std::cout << "\nentity:hasPosition=" << world.hasComponent<Position>(entity);
  std::cout << "\nentity:hasTag=" << world.hasComponent<Tag>(entity);
  std::cout << "\nentity:Tag=" << *world.getComponent<Tag>(entity);

  world.setComponent<double>(entity, INFINITY);
  world.emplaceComponent<Position>(entity, Vec2{100, 200});

  std::cout << "\n== after modifications (2) ==";
  std::cout << "\nentity:hasInt=" << world.hasComponent<int>(entity);
  std::cout << "\nentity:int=" << *world.getComponent<int>(entity);
  std::cout << "\nentity:hasDouble=" << world.hasComponent<double>(entity);
  std::cout << "\nentity:double=" << *world.getComponent<double>(entity);
  std::cout << "\nentity:hasPosition=" << world.hasComponent<Position>(entity);
  std::cout << "\nentity:Tag=" << *world.getComponent<Position>(entity);
  std::cout << "\nentity:hasTag=" << world.hasComponent<Tag>(entity);
  std::cout << "\nentity:Tag=" << *world.getComponent<Tag>(entity);

  {
    bad::World queryWorld;

    bad::EntityId player = queryWorld.create();
    queryWorld.emplaceComponent<Name>(player, "player");
    queryWorld.emplaceComponent<Player>(player);
    queryWorld.emplaceComponent<Position>(player, Vec2{0, 0});
    queryWorld.emplaceComponent<MoveIntention>(player, Vec2{1, 1});

    bad::EntityId rock = queryWorld.create();
    queryWorld.emplaceComponent<Name>(rock, "rock");
    queryWorld.emplaceComponent<Position>(rock, Vec2{3, 3});

    bad::EntityId building = queryWorld.create();
    queryWorld.emplaceComponent<Name>(building, "building");
    queryWorld.emplaceComponent<Position>(building, Vec2{5, 5});

    bad::EntityId enemy = queryWorld.create();
    queryWorld.emplaceComponent<Name>(enemy, "enemy");
    queryWorld.emplaceComponent<Position>(enemy, Vec2{10, 10});
    queryWorld.emplaceComponent<Velocity>(enemy, Vec2{2, 2});

    bad::EntityId arrow = queryWorld.create();
    queryWorld.emplaceComponent<Name>(arrow, "arrow");
    queryWorld.emplaceComponent<Position>(arrow, Vec2{4, 4});
    queryWorld.emplaceComponent<Velocity>(arrow, Vec2{5, 5});
    queryWorld.emplaceComponent<Ephemeral>(arrow);

    std::cout << "== manual queries  ==" << std::endl;
    queryWorld.forEach<Position, Velocity>(
        [](const auto& pos, const auto& vel) {
          std::cout << "position=" << pos << ", velocity=" << vel << std::endl;
        });
    queryWorld.forEach<Position, Velocity>(
        [](bad::EntityId entity, auto pos, auto vel) {
          std::cout << "entity=" << entity << " position=" << pos
                    << ", velocity=" << vel << std::endl;
        });
    queryWorld.forEach<Position>(
        [](auto pos) { std::cout << "position=" << pos << std::endl; });
    queryWorld.forEach<std::complex<double>>([](const auto& complex) {
      std::cout << "complex=" << complex << std::endl;
    });

    std::cout << "arrow:hasTag=" << queryWorld.hasComponent<Tag>(arrow) << '\n';
    bad::Commands commands;
    queryWorld.forEach<Name, Ephemeral>([&commands](bad::EntityId entity,
                                                    const auto&   name,
                                                    const auto&   ephemeral) {
      std::cout << "name=" << name << ", ephemeral=" << ephemeral << '\n';
      commands.setComponent(entity, Tag{});
    });
    std::cout << "arrow:hasTag=" << queryWorld.hasComponent<Tag>(arrow) << '\n';
    commands.execute(&queryWorld);
    std::cout << "arrow:hasTag=" << queryWorld.hasComponent<Tag>(arrow) << '\n';
  }

  return 0;
}