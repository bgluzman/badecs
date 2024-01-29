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
  std::cout << std::boolalpha;

  bad::ComponentStorage storage;
  bad::Entity           entity(1ULL, &storage);

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
  entity.add<Tag>();

  std::cout << "== after modifications (1) ==" << std::endl;
  intEntity = entity.get<int>();
  doubleEntity = entity.get<double>();
  tagEntity = entity.get<Tag>();
  std::cout << "entity:int=" << *intEntity << std::endl;
  std::cout << "entity:double=" << *doubleEntity << std::endl;
  std::cout << "entity:Tag=" << *tagEntity << std::endl;

  entity.set<double>(INFINITY);
  entity.add<Position>(100, 200);

  std::cout << "== after modifications (2) ==" << std::endl;
  intEntity = entity.get<int>();
  doubleEntity = entity.get<double>();
  positionEntity = entity.get<Position>();
  tagEntity = entity.get<Tag>();
  std::cout << "entity:hasInt=" << bool(intEntity) << std::endl;
  std::cout << "entity:hasDouble=" << bool(doubleEntity) << std::endl;
  std::cout << "entity:hasPosition=" << bool(positionEntity) << std::endl;
  std::cout << "entity:hasTag=" << bool(tagEntity) << std::endl;
  std::cout << "entity:int=" << *intEntity << std::endl;
  std::cout << "entity:double=" << *doubleEntity << std::endl;
  std::cout << "entity:Tag=" << *positionEntity << std::endl;
  std::cout << "entity:Tag=" << *tagEntity << std::endl;

  return 0;
}