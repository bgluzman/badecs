#pragma once

#include "Common.h"
#include "ComponentRegistry.h"
#include "System.h"

#include <concepts>
#include <vector>

namespace bad {

class SystemRegistry {
public:
  template <Component... Args>
  void add(SystemFunctor<Args...> auto&& system);

  void run(ComponentRegistry& components);

private:
  std::vector<System> systems_ = {};
};

template <Component... Args>
void SystemRegistry::add(SystemFunctor<Args...> auto&& system) {
  systems_.emplace_back(
      System::construct<Args...>(std::forward<decltype(system)>(system)));
}
}  // namespace bad