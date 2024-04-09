#include <badecs/internal/Column.h>

#include <gtest/gtest.h>

#include <ostream>

struct Position {
  int x;
  int y;
  bool operator<=>(const Position &) const = default;
};
std::ostream &operator<<(std::ostream &os, const Position &position) {
  return os << "Position{x=" << position.x << ", y=" << position.y << "}";
}

namespace bad::internal {

template <typename T>
testing::AssertionResult TestColumnValue(const Column &column,
                                         EntityId entityId, T reference) {
  auto *value = column.get(entityId);
  if (!value || !value->has_value()) {
    return testing::AssertionFailure()
           << "Column does not have a value for entityId " << entityId;
  }
  if (value->type() != typeid(T)) {
    return testing::AssertionFailure()
           << "Column has a value of the wrong type for entityId " << entityId;
  }
  if (std::any_cast<T>(*value) != reference) {
    return testing::AssertionFailure()
           << "Column has the wrong value for entityId " << entityId
           << " (expected " << reference << ", got " << std::any_cast<T>(*value)
           << ")";
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

  ASSERT_EQ(column.size(), 0);
  ASSERT_EQ(column.has(0), false);
  EXPECT_EQ(column.get(0), nullptr);

  // Initial emplacement.
  column.emplace<Position>(0, 1, 2);
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
  // Check that the value was emplaced.
  EXPECT_TRUE(TestColumnValue(column, 0, Position{1, 2}));
  // Check const-qualified get().
  const auto *const_column = &column;
  EXPECT_TRUE(TestColumnValue(*const_column, 0, Position{1, 2}));

  // Re-emplacement.
  column.emplace<Position>(0, 2, 3);
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
  // Check that the previous value was overwritten.
  EXPECT_TRUE(TestColumnValue(column, 0, Position{2, 3}));

  // Additional emplacement.
  column.emplace<Position>(1, 4, 5);
  EXPECT_EQ(column.has(1), true);
  EXPECT_EQ(column.size(), 2);
  // Check that the previous value was not overwritten.
  EXPECT_TRUE(TestColumnValue(column, 0, Position{2, 3}));
  // Check that new value was emplaced.
  EXPECT_TRUE(TestColumnValue(column, 1, Position{4, 5}));
}

TEST(ColumnTest, SetAndGet) {
  Column column;

  ASSERT_EQ(column.size(), 0);
  ASSERT_EQ(column.has(0), false);
  EXPECT_EQ(column.get(0), nullptr);

  // Initial set.
  column.set(0, 1);
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
  // Check that the value was set.
  EXPECT_TRUE(TestColumnValue(column, 0, 1));
  // Check const-qualified get().
  const auto *const_column = &column;
  EXPECT_TRUE(TestColumnValue(*const_column, 0, 1));

  // Re-set.
  column.set(0, 2);
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
  // Check that the previous value was overwritten.
  EXPECT_TRUE(TestColumnValue(column, 0, 2));

  // Additional set.
  column.set(1, 3);
  EXPECT_EQ(column.has(1), true);
  EXPECT_EQ(column.size(), 2);
  // Check that the previous value was not overwritten.
  EXPECT_TRUE(TestColumnValue(column, 0, 2));
  // Check that new value was emplaced.
  EXPECT_TRUE(TestColumnValue(column, 1, 3));
}

} // namespace bad::internal
