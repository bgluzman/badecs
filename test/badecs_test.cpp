#include <badecs/internal/Column.h>

#include <gtest/gtest.h>

#include <string>

namespace bad::internal {

template <typename Column, Component T>
testing::AssertionResult TestColumnValue(Column &&column, EntityId entityId,
                                         T reference) {
  auto *value = column.get(entityId);
  if (!value || !value->has_value()) {
    return testing::AssertionFailure()
           << "Column does not have a value for entityId " << entityId;
  }
  if (value->type() != typeid(T)) {
    return testing::AssertionFailure()
           << "Column has a value of the wrong type for entityId " << entityId;
  }
  if (std::any_cast<int>(*value) != reference) {
    return testing::AssertionFailure()
           << "Column has the wrong value for entityId " << entityId
           << " (expected " << reference << ", got "
           << std::any_cast<int>(*value) << ")";
  }
  return testing::AssertionSuccess();
}

TEST(ColumnTest, EmptyInvariants) {
  Column column;
  EXPECT_EQ(column.size(), 0);
  EXPECT_EQ(column.has(0), false);
}

TEST(ColumnTest, EmplaceAndGet) {
  Column column;
  const Column &const_column = column;

  ASSERT_EQ(column.size(), 0);
  ASSERT_EQ(column.has(0), false);
  EXPECT_EQ(column.get(0), nullptr);

  // Initial emplacement.
  column.emplace<int>(0, 1);
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
  // Check that the value was emplaced.
  EXPECT_TRUE(TestColumnValue(column, 0, 1));
  EXPECT_TRUE(TestColumnValue(const_column, 0, 1));

  // Re-emplacement.
  column.emplace<int>(0, 2);
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
  // Check that the previous value was overwritten.
  EXPECT_TRUE(TestColumnValue(column, 0, 2));
  EXPECT_TRUE(TestColumnValue(const_column, 0, 2));

  // Additional emplacement.
  column.emplace<int>(1, 3);
  EXPECT_EQ(column.has(1), true);
  EXPECT_EQ(column.size(), 2);
  // Check that the previous value was not overwritten.
  EXPECT_TRUE(TestColumnValue(column, 0, 2));
  EXPECT_TRUE(TestColumnValue(const_column, 0, 2));
  // Check that new value was emplaced.
  EXPECT_TRUE(TestColumnValue(column, 1, 3));
  EXPECT_TRUE(TestColumnValue(const_column, 1, 3));
}

} // namespace bad::internal
