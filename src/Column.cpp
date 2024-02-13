#include "Column.h"

namespace bad {

std::vector<EntityId> Column::getEntityIds() const {
  std::vector<EntityId> entityIds;
  entityIds.reserve(components_.size());
  std::transform(components_.begin(), components_.end(),
                 std::back_inserter(entityIds),
                 [](const auto& kv) { return kv.first; });
  std::sort(entityIds.begin(), entityIds.end());
  return entityIds;
}

}  // namespace bad