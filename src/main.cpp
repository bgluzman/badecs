#include "World.h"

#include <iostream>

template <typename T>
void printOptional(const std::optional<gsl::not_null<T *>>& o) {
  if (o) {
    std::cout << **o;
  } else {
    std::cout << "<null>";
  }
}

int main(int /*argc*/, char * /*argv*/[]) {
  bad::ComponentStorage storage;
  bad::Entity           entity(1ULL, &storage);

  entity.set<int>(42);
  entity.set<double>(NAN);

  std::cout << "entity:int=";
  printOptional(entity.get<int>());
  std::cout << std::endl;
  std::cout << "entity:double=";
  printOptional(entity.get<double>());
  std::cout << std::endl;

  entity.set<double>(INFINITY);

  std::cout << "entity:int=";
  printOptional(entity.get<int>());
  std::cout << std::endl;
  std::cout << "entity:double=";
  printOptional(entity.get<double>());
  std::cout << std::endl;
  return 0;
}