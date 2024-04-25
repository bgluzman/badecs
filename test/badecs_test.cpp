#include <badecs/View.h>
#include <badecs/internal/Column.h>
#include <badecs/internal/Components.h>
#include <badecs/internal/Entities.h>
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
testing::AssertionResult TestColumnValue(const Column& column,
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

TEST(ColumnTest, Remove) {
  Column column;

  // Initialize column.
  column.emplace<Position>(0, 1, 2);
  column.emplace<Position>(1, 3, 4);
  column.emplace<Position>(2, 5, 6);
  ASSERT_EQ(column.size(), 3);

  // Test removal of non-existent entity.
  ASSERT_FALSE(column.has(3));
  EXPECT_EQ(column.remove(3), false);
  EXPECT_EQ(column.size(), 3);

  // Test removal of existing entity.
  ASSERT_TRUE(column.has(1));
  ASSERT_TRUE(TestColumnValue(column, 1, Position{3, 4}));
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

TEST(ComponentsTest, Set) {
  Components components;

  ASSERT_EQ(components.has<int>(0), false);
  EXPECT_EQ(components.get<int>(0), nullptr);

  // Initial set.
  components.set(0, 1);
  EXPECT_EQ(components.has<int>(0), true);
  // Check that the value was set.
  EXPECT_TRUE(TestComponentValue(components, 0, 1));
  // Check const-qualified get().
  const auto *const_column = &components;
  EXPECT_TRUE(TestComponentValue(*const_column, 0, 1));

  // Re-set.
  components.set(0, 2);
  EXPECT_EQ(components.has<int>(0), true);
  // Check that the previous value was overwritten.
  EXPECT_TRUE(TestComponentValue(components, 0, 2));

  // Additional set.
  components.set(1, 3);
  EXPECT_EQ(components.has<int>(1), true);
  // Check that the previous value was not overwritten.
  EXPECT_TRUE(TestComponentValue(components, 0, 2));
  // Check that new value was emplaced.
  EXPECT_TRUE(TestComponentValue(components, 1, 3));
}

TEST(ComponentsTest, Remove) {
  Components components;

  // Initialize components.
  components.emplace<Position>(0, 1, 2);
  components.emplace<Position>(1, 3, 4);
  components.emplace<Position>(2, 5, 6);

  // Test removal of non-existent component type.
  ASSERT_FALSE(components.has<int>(0));
  EXPECT_EQ(components.remove<int>(0), false);

  // Test removal of non-existent entity.
  ASSERT_FALSE(components.has<Position>(3));
  EXPECT_EQ(components.remove<Position>(3), false);

  // Test removal of existing entity.
  ASSERT_TRUE(components.has<Position>(1));
  ASSERT_TRUE(TestComponentValue(components, 1, Position{3, 4}));
  EXPECT_EQ(components.remove<Position>(1), true);  // remove item
  EXPECT_EQ(components.has<Position>(1), false);
  EXPECT_EQ(components.get<Position>(1), nullptr);
}

TEST(ComponentsTest, RemoveAll) {
  Components components;

  // Initialize components.
  components.emplace<Position>(0, 1, 2);
  components.emplace<Position>(1, 3, 4);
  components.emplace<Position>(2, 5, 6);

  // Test removal of a set of entities.
  ASSERT_TRUE(components.has<Position>(0));
  ASSERT_TRUE(TestComponentValue(components, 0, Position{1, 2}));
  ASSERT_TRUE(components.has<Position>(1));
  ASSERT_TRUE(TestComponentValue(components, 1, Position{3, 4}));
  ASSERT_TRUE(components.has<Position>(2));
  ASSERT_TRUE(TestComponentValue(components, 2, Position{5, 6}));
  ASSERT_EQ(components.has<Position>(10), false);
  ASSERT_EQ(components.get<Position>(10), nullptr);
  components.removeAll<Position>(std::vector{0U, 2U, 10U});
  EXPECT_EQ(components.has<Position>(0), false);
  EXPECT_EQ(components.get<Position>(0), nullptr);
  ASSERT_TRUE(components.has<Position>(1));
  EXPECT_TRUE(TestComponentValue(components, 1, Position{3, 4}));
  EXPECT_EQ(components.has<Position>(2), false);
  EXPECT_EQ(components.get<Position>(2), nullptr);
  EXPECT_EQ(components.has<Position>(10), false);
  EXPECT_EQ(components.get<Position>(10), nullptr);
}

TEST(EntitiesTest, ReserveInstantiate) {
  Entities entities;

  // Reserve two entities before instantiating them.
  EntityId id1 = entities.reserve();
  EntityId id2 = entities.reserve();
  // Ensure reservations are unique.
  EXPECT_NE(id1, id2);
  EXPECT_FALSE(entities.has(id1));
  EXPECT_FALSE(entities.has(id2));

  // Instantiate the first entity.
  entities.instantiate(id1);
  EXPECT_TRUE(entities.has(id1));
  EXPECT_FALSE(entities.has(id2));

  // Instantiate the second entity.
  entities.instantiate(id2);
  EXPECT_TRUE(entities.has(id1));
  EXPECT_TRUE(entities.has(id2));

  // Now instantiate a third entity and ensure it's unique w.r.t. the first two.
  EntityId id3 = entities.reserve();
  entities.instantiate(id3);
  EXPECT_NE(id1, id3);
  EXPECT_NE(id2, id3);
  EXPECT_TRUE(entities.has(id3));
}

TEST(EntitiesTest, EntityComponents) {
  Entities entities;

  // Ensure non-existent entity has no components.
  EXPECT_FALSE(entities.hasComponent(0, componentId<Position>));
  // Ensure adding a component to a non-existent entity fails.
  EXPECT_FALSE(entities.addComponent(0, componentId<Position>));

  // Reserve and instantiate an entity.
  EntityId id = entities.reserve();
  entities.instantiate(id);
  EXPECT_TRUE(entities.has(id));
  EXPECT_FALSE(entities.hasComponent(id, componentId<Position>));

  // Add a component to the entity and check for its existence.
  entities.addComponent(id, componentId<Position>);
  EXPECT_TRUE(entities.hasComponent(id, componentId<Position>));

  // Remove the component and check for its absence.
  entities.removeComponent(id, componentId<Position>);
  EXPECT_FALSE(entities.hasComponent(id, componentId<Position>));
}

TEST(EntitiesTest, Remove) {
  Entities                                       entities;
  std::optional<std::unordered_set<ComponentId>> components = std::nullopt;

  // Try removing an entity that doesn't even exist.
  ASSERT_FALSE(entities.has(0));
  components = entities.remove(0);
  EXPECT_FALSE(components.has_value());

  // Reserve and instantiate an entity.
  EntityId id = entities.reserve();
  entities.instantiate(id);
  ASSERT_TRUE(entities.has(id));

  // Remove the entity before adding any components.
  components = entities.remove(id);
  EXPECT_TRUE(components.has_value());  // should be empty, but existent
  EXPECT_FALSE(entities.has(id));

  // Remove an entity with components.
  id = entities.reserve();
  entities.instantiate(id);
  ASSERT_TRUE(entities.has(id));
  ASSERT_TRUE(entities.addComponent(id, componentId<Position>));
  ASSERT_TRUE(entities.hasComponent(id, componentId<Position>));
  components = entities.remove(id);
  ASSERT_TRUE(components.has_value());
  ASSERT_EQ(components->size(), 1);
  EXPECT_EQ(*components->begin(), componentId<Position>);
  EXPECT_FALSE(entities.has(id));
}

class ViewTest : public testing::Test {
protected:
  void SetUp() override {
    posColumn.emplace<Position>(0, 1, 2);
    posColumn.emplace<Position>(1, 3, 4);
    posColumn.emplace<Position>(2, 5, 6);
    intColumn.set(1, 42);
    boolColumn.set(0, true);
    boolColumn.set(2, false);
    floatColumn.set(2, 123.f);
  }

  Column posColumn, intColumn, boolColumn, floatColumn;
};

TEST_F(ViewTest, EmptyView) {
  View<Position> nullView1({nullptr});
  EXPECT_EQ(nullView1.begin(), nullView1.end());
  View<Position, int> nullView2({&posColumn, nullptr});
  EXPECT_EQ(nullView2.begin(), nullView2.end());

  Column      emptyColumn;
  View<float> emptyColumnView1({&emptyColumn});
  EXPECT_EQ(emptyColumnView1.begin(), emptyColumnView1.end());
  View<Position, float> emptyColumnView2({&posColumn, &emptyColumn});
  EXPECT_EQ(emptyColumnView2.begin(), emptyColumnView2.end());
}

TEST_F(ViewTest, SingleComponent) {
  View<Position> view({&posColumn});
  auto           begin = view.begin();
  auto           end = view.end();

  EXPECT_EQ(std::distance(begin, end), posColumn.size());

  for (auto [columnKV, viewTuple] : std::views::zip(posColumn, view)) {
    auto [columnEntityId, posAny] = columnKV;
    auto [viewEntityId, viewPos] = viewTuple;
    SCOPED_TRACE("columnEntityId=" + std::to_string(columnEntityId) +
                 ", viewEntityId=" + std::to_string(viewEntityId));
    ASSERT_TRUE(posAny.has_value());
    ASSERT_EQ(posAny.type(), typeid(Position));
    EXPECT_EQ(columnEntityId, viewEntityId);
    EXPECT_EQ(std::any_cast<Position>(posAny), viewPos);
  }
}

TEST_F(ViewTest, MultiComponent) {
  View<Position, bool> view({&posColumn, &boolColumn});

  // Construct a map of the relationships from the view since iteration order
  // is not guaranteed.
  std::map<EntityId, std::tuple<Position, bool>> viewData;
  std::transform(view.begin(), view.end(),
                 std::inserter(viewData, viewData.end()),
                 [](const auto& tuple) {
                   auto [entityId, pos, b] = tuple;
                   return std::make_pair(entityId, std::make_tuple(pos, b));
                 });

  EXPECT_EQ(viewData.size(), 2);

  ASSERT_TRUE(viewData.contains(0));
  EXPECT_EQ(std::get<0>(viewData[0]), (Position{1, 2}));
  EXPECT_EQ(std::get<1>(viewData[0]), true);

  ASSERT_TRUE(viewData.contains(2));
  EXPECT_EQ(std::get<0>(viewData[2]), (Position{5, 6}));
  EXPECT_EQ(std::get<1>(viewData[2]), false);
}

TEST_F(ViewTest, EmptyIntersection) {
  // `intColumn` and `boolColumn` have disjoint sets of entities.
  View<Position, int, bool> view({&posColumn, &intColumn, &boolColumn});
  EXPECT_EQ(view.begin(), view.end());
}

TEST_F(ViewTest, FilterViewedColumn) {
  View<Position, bool> view({&posColumn, &boolColumn});
  // Filtering on any column which is viewed results in no values iterated.
  view.filterColumn(&boolColumn);
  EXPECT_EQ(view.begin(), view.end());
}

TEST_F(ViewTest, FilterDisjointColumn) {
  View<Position, bool> view({&posColumn, &boolColumn});
  // Filtering on a column which is disjoint w.r.t. the viewed columns should
  // not impact the values iterated.
  view.filterColumn(&intColumn);
  EXPECT_EQ(std::distance(view.begin(), view.end()), 2);
}

TEST_F(ViewTest, FilterOverlappingColumn) {
  View<Position, bool> view({&posColumn, &boolColumn});
  // Filtering on a column which overlaps with the viewed columns should
  // cause those entities to not appear when being iterated. Here, that is
  // entity with id=2.
  view.filterColumn(&floatColumn);
  EXPECT_EQ(std::distance(view.begin(), view.end()), 1);
  // Only entity with id=0 should remain.
  auto [entityId, pos, b] = *view.begin();
  EXPECT_EQ(entityId, 0);
  EXPECT_EQ(pos, (Position{1, 2}));
  EXPECT_EQ(b, true);
}

}  // namespace bad::internal
