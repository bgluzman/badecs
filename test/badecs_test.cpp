#include <badecs/Registry.h>

#include <gtest/gtest.h>

namespace bad {

TEST(RegistryTest, Stub) {
  Registry registry;
  EXPECT_EQ(registry.stub(), 42);
}

} // namespace bad
