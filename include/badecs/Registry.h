#ifndef BADECS_REGISTRY_H
#define BADECS_REGISTRY_H

namespace bad {

/// @brief The top-level registry of entity-component relationships.
///
/// Maintains entities and their constituent components. Allows creation,
/// modification, and deletion of entities/components along with querying
/// for entities with specific components.
class Registry {
public:
  int stub() const { return 42; }
};

} // namespace bad

#endif // BADECS_REGISTRY_H
