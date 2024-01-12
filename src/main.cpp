#include "World.h"

#include <iostream>

int main(int /*argc*/, char * /*argv*/[]) {
  bad::ComponentStorage storage;
  bad::Entity           entity(1ULL, &storage);

  entity.set<int>(42);
  entity.set<double>(NAN);

  std::cout << "entity:int=" << entity.get<int>() << std::endl;
  std::cout << "entity:double=" << entity.get<double>() << std::endl;

  entity.set<double>(INFINITY);

  std::cout << "entity:int=" << entity.get<int>() << std::endl;
  std::cout << "entity:double=" << entity.get<double>() << std::endl;
  return 0;
}