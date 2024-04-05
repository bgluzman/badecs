#ifndef BADECS_REGISTRY_H
#define BADECS_REGISTRY_H

namespace badecs {

/// @brief The top-level registry of entity-component relationship.
///
/// Maintains entities and their constituent components. Allows creation,
/// modification, and deletion of entities/components along with querying
/// for entities with specific components.
class Registry {
public:
  int stub() const { return 42; }
};

} // namespace badecs

#endif // BADECS_REGISTRY_H
