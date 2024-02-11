#include "World.h"

#include <cmath>
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

  bad::World        world;
  bad::EntityHandle entity = world.entity();

  auto intEntity = entity.get<int>();
  auto doubleEntity = entity.get<double>();
  auto positionEntity = entity.get<Position>();
  auto tagEntity = entity.get<Tag>();

  std::cout << "== before ==" << std::endl;
  std::cout << "entity:hasInt=" << bool(intEntity) << std::endl;
  std::cout << "entity:hasDouble=" << bool(doubleEntity) << std::endl;
  std::cout << "entity:hasPosition=" << bool(positionEntity) << std::endl;
  std::cout << "entity:hasTag=" << bool(tagEntity) << std::endl;

  entity.set<int>(42);
  entity.set<double>(NAN);
  entity.emplace<Tag>();

  std::cout << "== after modifications (1) ==" << std::endl;
  intEntity = entity.get<int>();
  doubleEntity = entity.get<double>();
  positionEntity = entity.get<Position>();
  tagEntity = entity.get<Tag>();
  std::cout << "entity:hasInt=" << bool(intEntity) << std::endl;
  std::cout << "entity:int=" << **intEntity << std::endl;
  std::cout << "entity:hasDouble=" << bool(doubleEntity) << std::endl;
  std::cout << "entity:double=" << **doubleEntity << std::endl;
  std::cout << "entity:hasPosition=" << bool(positionEntity) << std::endl;
  std::cout << "entity:hasTag=" << bool(tagEntity) << std::endl;
  std::cout << "entity:Tag=" << **tagEntity << std::endl;

  entity.set<double>(INFINITY);
  entity.emplace<Position>(Vec2{100, 200});

  std::cout << "== after modifications (2) ==" << std::endl;
  intEntity = entity.get<int>();
  doubleEntity = entity.get<double>();
  positionEntity = entity.get<Position>();
  tagEntity = entity.get<Tag>();
  std::cout << "entity:hasInt=" << bool(intEntity) << std::endl;
  std::cout << "entity:int=" << **intEntity << std::endl;
  std::cout << "entity:hasDouble=" << bool(doubleEntity) << std::endl;
  std::cout << "entity:double=" << **doubleEntity << std::endl;
  std::cout << "entity:hasPosition=" << bool(positionEntity) << std::endl;
  std::cout << "entity:Tag=" << **positionEntity << std::endl;
  std::cout << "entity:hasTag=" << bool(tagEntity) << std::endl;
  std::cout << "entity:Tag=" << **tagEntity << std::endl;

  {
    bad::World queryWorld;

    bad::EntityHandle player = queryWorld.entity();
    player.emplace<Name>("player");
    player.emplace<Player>();
    player.emplace<Position>(Vec2{0, 0});
    player.emplace<MoveIntention>(Vec2{1, 1});

    bad::EntityHandle rock = queryWorld.entity();
    rock.emplace<Name>("rock");
    rock.emplace<Position>(Vec2{3, 3});

    bad::EntityHandle building = queryWorld.entity();
    building.emplace<Name>("building");
    building.emplace<Position>(Vec2{5, 5});

    bad::EntityHandle enemy = queryWorld.entity();
    enemy.emplace<Name>("enemy");
    enemy.emplace<Position>(Vec2{10, 10});
    enemy.emplace<Velocity>(Vec2{2, 2});

    bad::EntityHandle arrow = queryWorld.entity();
    arrow.emplace<Name>("arrow");
    arrow.emplace<Position>(Vec2{4, 4});
    arrow.emplace<Velocity>(Vec2{5, 5});
    arrow.emplace<Ephemeral>();

    std::cout << "== manual queries  ==" << std::endl;
    queryWorld.query<Position, Velocity>([](const auto& pos, const auto& vel) {
      std::cout << "position=" << pos << ", velocity=" << vel << std::endl;
    });
    queryWorld.query<Position, Velocity>(
        [](bad::EntityHandle entity, auto pos, auto vel) {
          std::cout << "entity=" << entity.getId() << " position=" << pos
                    << ", velocity=" << vel << std::endl;
        });
  }

  return 0;
}