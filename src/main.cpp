#include "Commands.h"
#include "View.h"
#include "World.h"

#include <cassert>
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

    for (auto [id, name] : queryWorld.view<Name>()) {
      std::cout << "[sortedview1] entity=" << id << ", name=" << name << '\n';
    }
    for (auto [id, name, pos] : queryWorld.view<Name, const Position&>()) {
      std::cout << "[sortedview2] entity=" << id << ", name=" << name
                << ", position=" << pos << '\n';
    }
    for (auto [id, name, pos, _] : queryWorld.view<Name, Position&, Player>()) {
      std::cout << "[sortedview3] id:isRef="
                << std::is_reference_v<decltype(id)> << ", name:isRef="
                << std::is_reference_v<decltype(name)> << ", pos:isRef="
                << std::is_reference_v<decltype(pos)> << ", player:isRef="
                << std::is_reference_v<decltype(_)> << '\n';
      std::cout << "[sortedview3] entity=" << id << ", name=" << name
                << ", position=" << pos << '\n';
    }
    for (auto [id, name, _] : queryWorld.view<Name, std::complex<float>>()) {
      std::cout << "[sortedview4] entity=" << id << ", name=" << name << '\n';
    }
    std::cout << "---" << std::endl;
    queryWorld.emplaceComponent<Tag>(building);
    for (auto [id, name] : queryWorld.view<Name>(bad::filter<>)) {
      std::cout << "[sortedview5] entity=" << id << ", name=" << name << '\n';
    }
    for (auto [id, name] : queryWorld.view<Name>(bad::filter<MoveIntention>)) {
      std::cout << "[sortedview6] entity=" << id << ", name=" << name << '\n';
    }
    for (auto [id, name] :
         queryWorld.view<Name>(bad::filter<MoveIntention, Tag>)) {
      std::cout << "[sortedview7] entity=" << id << ", name=" << name << '\n';
    }
  }

  return 0;
}