#pragma once

#include "Common.h"

#include <any>
#include <memory>
#include <unordered_map>

namespace bad {

struct Column {
  std::unordered_map<EntityId, std::unique_ptr<std::any>> components = {};
};

}  // namespace bad
