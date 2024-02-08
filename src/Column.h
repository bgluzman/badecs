#pragma once

#include "Common.h"

#include <algorithm>
#include <any>
#include <memory>
#include <unordered_map>
#include <vector>

namespace bad {

struct Column {
  std::unordered_map<EntityId, std::unique_ptr<std::any>> components = {};

  std::vector<EntityId> getEntityIds() {
    std::vector<EntityId> entityIds;
    entityIds.reserve(components.size());
    std::transform(components.begin(), components.end(),
                   std::back_inserter(entityIds),
                   [](const auto& kv) { return kv.first; });
    std::sort(entityIds.begin(), entityIds.end());
    return entityIds;
  }
};

}  // namespace bad
