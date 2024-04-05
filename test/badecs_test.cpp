#include <badecs/Registry.h>

#include <gtest/gtest.h>

namespace badecs {

TEST(RegistryTest, Stub) {
  Registry registry;
  EXPECT_EQ(registry.stub(), 42);
}

} // namespace badecs
