#include "World.h"

#include <cmath>
#include <iostream>

struct Position {
  int x;
  int y;
};

std::ostream& operator<<(std::ostream& os, const Position& position) {
  return os << "Position{x=" << position.x << ", y=" << position.y << '}';
}

struct Tag {};

std::ostream& operator<<(std::ostream& os, const Tag&) { return os << "Tag{}"; }

int main(int /*argc*/, char * /*argv*/[]) {
  bad::ComponentStorage storage;
  bad::Entity           entity(1ULL, &storage);

  std::cout << std::boolalpha;
  std::cout << "entity:hasInt=" << entity.has<int>() << std::endl;
  std::cout << "entity:hasDouble=" << entity.has<double>() << std::endl;
  std::cout << "entity:hasPosition=" << entity.has<Position>() << std::endl;
  std::cout << "entity:hasTag=" << entity.has<Tag>() << std::endl;

  entity.set<int>(42);
  entity.set<double>(NAN);
  entity.add<Tag>();

  std::cout << "entity:int=" << entity.get<int>() << std::endl;
  std::cout << "entity:double=" << entity.get<double>() << std::endl;
  std::cout << "entity:Tag=" << entity.get<Tag>() << std::endl;

  entity.set<double>(INFINITY);
  entity.add<Position>(100, 200);

  std::cout << "entity:hasInt=" << entity.has<int>() << std::endl;
  std::cout << "entity:hasDouble=" << entity.has<double>() << std::endl;
  std::cout << "entity:hasPosition=" << entity.has<Position>() << std::endl;
  std::cout << "entity:hasTag=" << entity.has<Tag>() << std::endl;

  std::cout << "entity:int=" << entity.get<int>() << std::endl;
  std::cout << "entity:double=" << entity.get<double>() << std::endl;
  std::cout << "entity:Position=" << entity.get<Position>() << std::endl;
  std::cout << "entity:Tag=" << entity.get<Tag>() << std::endl;

  return 0;
}