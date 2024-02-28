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

  world.set<int>(entity, 42);
  world.set<double>(entity, NAN);
  world.emplace<Tag>(entity);

  auto *intEntity = world.get<int>(entity);
  auto *doubleEntity = world.get<double>(entity);
  auto *positionEntity = world.get<Position>(entity);
  auto *tagEntity = world.get<Tag>(entity);

  std::cout << "== before modifications ==" << std::endl;
  std::cout << "entity:int=" << *intEntity << std::endl;
  std::cout << "entity:double=" << *doubleEntity << std::endl;
  std::cout << "entity:Tag=" << *tagEntity << std::endl;

  world.set<double>(entity, INFINITY);
  world.emplace<Position>(entity, Vec2{100, 200});

  std::cout << "== after modifications ==" << std::endl;
  intEntity = world.get<int>(entity);
  doubleEntity = world.get<double>(entity);
  positionEntity = world.get<Position>(entity);
  tagEntity = world.get<Tag>(entity);
  std::cout << "entity:int=" << *intEntity << std::endl;
  std::cout << "entity:double=" << *doubleEntity << std::endl;
  std::cout << "entity:Tag=" << *positionEntity << std::endl;
  std::cout << "entity:Tag=" << *tagEntity << std::endl;

  {
    bad::World queryWorld;

    bad::EntityId player = queryWorld.create();
    queryWorld.emplace<Name>(player, "player");
    queryWorld.emplace<Player>(player);
    queryWorld.emplace<Position>(player, Vec2{0, 0});
    queryWorld.emplace<MoveIntention>(player, Vec2{1, 1});

    bad::EntityId rock = queryWorld.create();
    queryWorld.emplace<Name>(rock, "rock");
    queryWorld.emplace<Position>(rock, Vec2{3, 3});

    bad::EntityId building = queryWorld.create();
    queryWorld.emplace<Name>(building, "building");
    queryWorld.emplace<Position>(building, Vec2{5, 5});

    bad::EntityId enemy = queryWorld.create();
    queryWorld.emplace<Name>(enemy, "enemy");
    queryWorld.emplace<Position>(enemy, Vec2{10, 10});
    queryWorld.emplace<Velocity>(enemy, Vec2{2, 2});

    bad::EntityId arrow = queryWorld.create();
    queryWorld.emplace<Name>(arrow, "arrow");
    queryWorld.emplace<Position>(arrow, Vec2{4, 4});
    queryWorld.emplace<Velocity>(arrow, Vec2{5, 5});
    queryWorld.emplace<Ephemeral>(arrow);

    std::cout << "== manual queries  ==" << std::endl;
    queryWorld.query<Position, Velocity>([](const auto& pos, const auto& vel) {
      std::cout << "position=" << pos << ", velocity=" << vel << std::endl;
    });
    queryWorld.query<Position, Velocity>(
        [](bad::EntityId entity, auto pos, auto vel) {
          std::cout << "entity=" << entity << " position=" << pos
                    << ", velocity=" << vel << std::endl;
        });
    queryWorld.query<Position>(
        [](auto pos) { std::cout << "position=" << pos << std::endl; });
    queryWorld.query<std::complex<double>>([](const auto& complex) {
      std::cout << "complex=" << complex << std::endl;
    });
  }

  return 0;
}