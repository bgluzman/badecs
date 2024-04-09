#include <badecs/internal/Column.h>
#include <badecs/internal/Components.h>
#include <gtest/gtest.h>
#include <map>
#include <ostream>

struct Position {
  int  x;
  int  y;
  auto operator<=>(const Position&) const = default;
};
std::ostream& operator<<(std::ostream& os, const Position& position) {
  return os << "Position{x=" << position.x << ", y=" << position.y << "}";
}

namespace bad::internal {

template <typename T>
testing::AssertionResult TestComponentValue(const Column& column,
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
  EXPECT_EQ(column.get(0), nullptr);
}

TEST(ColumnTest, Emplace) {
  Column column;

  ASSERT_EQ(column.size(), 0);
  ASSERT_EQ(column.has(0), false);
  EXPECT_EQ(column.get(0), nullptr);

  // Initial emplacement.
  column.emplace<Position>(0, 1, 2);
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
  // Check that the value was emplaced.
  EXPECT_TRUE(TestComponentValue(column, 0, Position{1, 2}));
  // Check const-qualified get().
  const auto *const_column = &column;
  EXPECT_TRUE(TestComponentValue(*const_column, 0, Position{1, 2}));

  // Re-emplacement.
  column.emplace<Position>(0, 2, 3);
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
  // Check that the previous value was overwritten.
  EXPECT_TRUE(TestComponentValue(column, 0, Position{2, 3}));

  // Additional emplacement.
  column.emplace<Position>(1, 4, 5);
  EXPECT_EQ(column.has(1), true);
  EXPECT_EQ(column.size(), 2);
  // Check that the previous value was not overwritten.
  EXPECT_TRUE(TestComponentValue(column, 0, Position{2, 3}));
  // Check that new value was emplaced.
  EXPECT_TRUE(TestComponentValue(column, 1, Position{4, 5}));
}

TEST(ColumnTest, Set) {
  Column column;

  ASSERT_EQ(column.size(), 0);
  ASSERT_EQ(column.has(0), false);
  EXPECT_EQ(column.get(0), nullptr);

  // Initial set.
  column.set(0, 1);
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
  // Check that the value was set.
  EXPECT_TRUE(TestComponentValue(column, 0, 1));
  // Check const-qualified get().
  const auto *const_column = &column;
  EXPECT_TRUE(TestComponentValue(*const_column, 0, 1));

  // Re-set.
  column.set(0, 2);
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
  // Check that the previous value was overwritten.
  EXPECT_TRUE(TestComponentValue(column, 0, 2));

  // Additional set.
  column.set(1, 3);
  EXPECT_EQ(column.has(1), true);
  EXPECT_EQ(column.size(), 2);
  // Check that the previous value was not overwritten.
  EXPECT_TRUE(TestComponentValue(column, 0, 2));
  // Check that new value was emplaced.
  EXPECT_TRUE(TestComponentValue(column, 1, 3));
}

TEST(ColumnTest, Remove) {
  Column column;

  // Initialize column.
  column.emplace<Position>(0, 1, 2);
  column.emplace<Position>(1, 3, 4);
  column.emplace<Position>(2, 3, 4);
  ASSERT_EQ(column.size(), 3);

  // Test removal of non-existent entity.
  ASSERT_FALSE(column.has(3));
  EXPECT_EQ(column.remove(3), false);
  EXPECT_EQ(column.size(), 3);

  // Test removal of existing entity.
  ASSERT_TRUE(column.has(1));
  ASSERT_TRUE(TestComponentValue(column, 1, Position{3, 4}));
  EXPECT_EQ(column.remove(1), true);  // remove item
  EXPECT_EQ(column.size(), 2);
  EXPECT_EQ(column.has(1), false);
  EXPECT_EQ(column.get(1), nullptr);
}

TEST(ColumnTest, Iterators) {
  Column column;
  // Initialize a map of test positions to whether they have been found in the
  // column. We will iterate over the column and mark each position as found.
  std::map<Position, bool> positions = {
      {Position{1, 2}, false},
      {Position{3, 4}, false},
      {Position{5, 6}, false},
  };

  // Initialize column.
  EntityId id = 0;
  for (const auto& [pos, _] : positions) {
    column.set(id++, pos);
  }
  ASSERT_EQ(column.size(), 3);

  // Iterate over the column and mark each position as found if it has not
  // already been seen. If it has been seen, then fail.
  for (const auto& [entityId, value] : column) {
    auto pos = std::any_cast<Position>(value);
    auto lookup = positions.find(pos);
    ASSERT_NE(lookup, positions.end()) << "Unexpected position found " << pos;
    if (lookup->second) {
      FAIL() << "Position " << lookup->first << " was found twice";
    }
    lookup->second = true;
  }
}

template <typename T>
testing::AssertionResult TestComponentValue(const Components& components,
                                            EntityId entityId, T reference) {
  const T *value = components.get<T>(entityId);
  if (!value) {
    return testing::AssertionFailure()
           << "Component does not have a value for entityId " << entityId;
  }
  if (*value != reference) {
    return testing::AssertionFailure()
           << "Component has the wrong value for entityId " << entityId
           << " (expected " << reference << ", got " << *value << ")";
  }
  return testing::AssertionSuccess();
}

TEST(Components, EmptyInvariants) {
  Components components;
  EXPECT_EQ(components.has<Position>(0), false);
  EXPECT_EQ(components.get<Position>(0), nullptr);
}

TEST(Components, Emplace) {
  Components components;

  ASSERT_EQ(components.has<Position>(0), false);
  EXPECT_EQ(components.get<Position>(0), nullptr);

  // Initial emplacement.
  components.emplace<Position>(0, 1, 2);
  EXPECT_EQ(components.has<Position>(0), true);
  // Check that the value was emplaced.
  EXPECT_TRUE(TestComponentValue(components, 0, Position{1, 2}));
  // Check const-qualified get().
  const auto *const_column = &components;
  EXPECT_TRUE(TestComponentValue(*const_column, 0, Position{1, 2}));

  // Re-emplacement.
  components.emplace<Position>(0, 2, 3);
  EXPECT_EQ(components.has<Position>(0), true);
  // Check that the previous value was overwritten.
  EXPECT_TRUE(TestComponentValue(components, 0, Position{2, 3}));

  // Additional emplacement.
  components.emplace<Position>(1, 4, 5);
  EXPECT_EQ(components.has<Position>(1), true);
  // Check that the previous value was not overwritten.
  EXPECT_TRUE(TestComponentValue(components, 0, Position{2, 3}));
  // Check that new value was emplaced.
  EXPECT_TRUE(TestComponentValue(components, 1, Position{4, 5}));
}

}  // namespace bad::internal
