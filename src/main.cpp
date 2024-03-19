#include "Commands.h"
#include "Query.h"
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

    std::cout << "\n== manual queries  ==" << std::endl;

    // syntax using .with()
    bad::query(&queryWorld)
        .with<Position>()
        .with<Velocity>()
        .each([](auto pos, auto vel) {
          std::cout << "position=" << pos << ", velocity=" << vel << std::endl;
        });
    bad::query(&queryWorld)
        .with<Position>()
        .with<Velocity>()
        .eachE([](bad::EntityId entity, const auto& pos, auto& vel) {
          std::cout << "entity=" << entity << " position=" << pos
                    << ", velocity=" << vel << std::endl;
          vel.value.x += 1;
        });
    bad::query(&queryWorld)
        .with<std::complex<double>>()
        .each([](const auto& complex) {
          std::cout << "complex=" << complex << std::endl;
        });

    // syntax using query<...> template params
    bad::query<Name>(&queryWorld).each([](const auto& name) {
      std::cout << "name=" << name << std::endl;
    });
    bad::query<Name, Position>(&queryWorld)
        .each([](const auto& name, const auto& position) {
          std::cout << "name=" << name << ", position=" << position
                    << std::endl;
        });
    bad::query<Name, Position, Velocity>(&queryWorld)
        .each([](const auto& name, const auto& position, const auto& velocity) {
          std::cout << "name=" << name << ", position=" << position
                    << ", velocity=" << velocity << std::endl;
        });

    std::cout << "arrow:hasTag=" << queryWorld.hasComponent<Tag>(arrow) << '\n';
    bad::Commands commands(&queryWorld);
    bad::query(&queryWorld)
        .with<Name>()
        .with<Ephemeral>()
        .eachE([&commands](bad::EntityId entity, const auto& name,
                           const auto& ephemeral) {
          std::cout << "name=" << name << ", ephemeral=" << ephemeral << '\n';
          commands.setComponent(entity, Tag{});
        });
    std::cout << "arrow:hasTag=" << queryWorld.hasComponent<Tag>(arrow) << '\n';
    commands.execute();
    std::cout << "arrow:hasTag=" << queryWorld.hasComponent<Tag>(arrow) << '\n';

    std::cout << "=== removeComponent test === \n";
    std::cout << "enemy:hasVelocity="
              << queryWorld.hasComponent<Velocity>(enemy) << '\n';
    bad::query<Name, Position>(&queryWorld)
        .filter<Velocity>()
        .each([](const auto& name, const auto& pos) {
          std::cout << "name=" << name << ", position=" << pos << '\n';
        });
    assert(queryWorld.removeComponent<Velocity>(enemy));
    std::cout << "enemy:hasVelocity="
              << queryWorld.hasComponent<Velocity>(enemy) << '\n';
    bad::query(&queryWorld)
        .with<Name>()
        .with<Position>()
        .filter<Velocity>()
        .each([](const auto& name, const auto& pos) {
          std::cout << "name=" << name << ", position=" << pos << '\n';
        });
    std::cout << "enemy:hasVelocity="
              << queryWorld.hasComponent<Velocity>(enemy) << '\n';

    std::cout << "=== destroy test === \n";
    std::cout << "has:arrow=" << queryWorld.has(arrow) << '\n';
    bad::query<Name>(&queryWorld).each([](const auto& name) {
      std::cout << "name=" << name << '\n';
    });
    std::cout << "destroy:arrow=" << queryWorld.destroy(arrow) << '\n';
    std::cout << "has:arrow=" << queryWorld.has(arrow) << '\n';
    bad::query<Name>(&queryWorld).each([](const auto& name) {
      std::cout << "name=" << name << '\n';
    });
    std::cout << "has:arrow=" << queryWorld.has(arrow) << '\n';

    bad::query(&queryWorld).eachE([](auto entity) {
      std::cout << "entity=" << entity << '\n';
    });

    //    bad::query<Position>(&queryWorld)
    //        .eachEC(
    //            [](bad::EntityId entity, bad::Commands *commands, const auto&
    //            pos) {
    //              std::cout << "entity=" << entity << ", position=" << pos <<
    //              '\n'; commands->removeComponent<Position>(entity);
    //
    //              bad::EntityId newEntity = commands->create();
    //              commands->setComponent<Position>(newEntity, pos);
    //              commands->setComponent<Tag>(newEntity, {});
    //            });
    //    bad::query<Position>(&queryWorld)
    //        .eachE([&w = queryWorld](bad::EntityId entity, const auto& pos) {
    //          std::cout << "entity=" << entity << ", position=" << pos
    //                    << ", has:tag=" << w.hasComponent<Tag>(entity) <<
    //                    '\n';
    //        });
    //
    //    for (auto [id, value] : queryWorld.components<Position>()) {
    //      std::cout << "[as] entity=" << id << ", position=" << value << '\n';
    //    }

    auto col1 = queryWorld.getColumn<Name>();
    auto col2 = queryWorld.getColumn<Position>();
    auto col3 = queryWorld.getColumn<Player>();
    for (auto [id, name, pos] :
         bad::View<false, Name, Position>({col1, col2})) {
      std::cout << "[view] entity=" << id << ", name=" << name
                << ", position=" << pos << '\n';
    }
    for (auto [id, name] : bad::valueView<Name>({col1})) {
      std::cout << "[sortedview1] entity=" << id << ", name=" << name << '\n';
    }
    for (auto [id, name, pos] : bad::valueView<Name, Position>({col1, col2})) {
      std::cout << "[sortedview2] entity=" << id << ", name=" << name
                << ", position=" << pos << '\n';
    }
    for (auto [id, name, pos, _] :
         bad::refView<Name, Position, Player>({col1, col2, col3})) {
      std::cout << "[sortedview3] entity=" << id << ", name=" << name
                << ", position=" << pos << '\n';
    }
  }

  return 0;
}