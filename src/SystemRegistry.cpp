#include "SystemRegistry.h"

namespace bad {

void SystemRegistry::run(ComponentRegistry& components) {
  for (System& system : systems_) {
    for (EntityId id : system.getQueryComponents(components)) {
      system.execute(components, id);
    }
  }
}

}  // namespace bad