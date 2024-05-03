#include <badecs/Filter.h>
#include <badecs/Registry.h>
#include <badecs/View.h>
#include <badecs/internal/Column.h>
#include <badecs/internal/Components.h>
#include <badecs/internal/Entities.h>
#include <badecs/internal/ViewImpl.h>
#include <gtest/gtest.h>
#include <ostream>

struct Position {
  int  x;
  int  y;
  auto operator<=>(const Position&) const = default;
};
std::ostream& operator<<(std::ostream& os, const Position& position) {
  return os << "Position{x=" << position.x << ", y=" << position.y << "}";
}

namespace bad {
namespace internal {

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
  EXPECT_TRUE(TestColumnValue(std::as_const(column), 0, Position{1, 2}));

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
  EXPECT_TRUE(TestColumnValue(std::as_const(column), 0, 1));

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

  // Initialize column.
  column.emplace<Position>(0, 1, 2);
  column.emplace<Position>(1, 3, 4);
  column.emplace<Position>(2, 5, 6);
  ASSERT_EQ(column.size(), 3);

  // Ensure we see each expected entry exactly once.
  for (auto& [entityId, value] : column) {
    ASSERT_EQ(value.type(), (typeid(Position)));
    auto position = std::any_cast<Position>(value);
    switch (entityId) {
    case 0:
      EXPECT_EQ(position, (Position{1, 2}));
      break;
    case 1:
      EXPECT_EQ(position, (Position{3, 4}));
      break;
    case 2:
      EXPECT_EQ(position, (Position{5, 6}));
      break;
    default:
      FAIL() << "Unexpected entityId " << entityId;
    }
  }
}

class ViewImplTest : public testing::Test {
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

TEST_F(ViewImplTest, EmptyView) {
  ViewImpl<Position> emptyView;
  EXPECT_EQ(emptyView.begin(), emptyView.end());

  ViewImpl<Position> nullView1({nullptr});
  EXPECT_EQ(nullView1.begin(), nullView1.end());
  ViewImpl<Position, int> nullView2({&posColumn, nullptr});
  EXPECT_EQ(nullView2.begin(), nullView2.end());

  Column          emptyColumn;
  ViewImpl<float> emptyColumnView1({&emptyColumn});
  EXPECT_EQ(emptyColumnView1.begin(), emptyColumnView1.end());
  ViewImpl<Position, float> emptyColumnView2({&posColumn, &emptyColumn});
  EXPECT_EQ(emptyColumnView2.begin(), emptyColumnView2.end());
}

TEST_F(ViewImplTest, SingleComponent) {
  ViewImpl<Position> view({&posColumn});
  auto               begin = view.begin();
  auto               end = view.end();

  EXPECT_EQ(std::distance(begin, end), posColumn.size());

  for (auto [columnKV, viewTuple] : std::views::zip(posColumn, view)) {
    auto [columnEntityId, posAny] = columnKV;
    auto [viewEntityId, viewPos] = viewTuple;
    static_assert(std::is_same_v<decltype(viewEntityId), EntityId>,
                  "columnEntityId is not an EntityId");
    static_assert(std::is_same_v<decltype(viewPos), Position&>,
                  "viewPos has incorrect type");
    SCOPED_TRACE("columnEntityId=" + std::to_string(columnEntityId) +
                 ", viewEntityId=" + std::to_string(viewEntityId));
    ASSERT_TRUE(posAny.has_value());
    ASSERT_EQ(posAny.type(), typeid(Position));
    EXPECT_EQ(columnEntityId, viewEntityId);
    EXPECT_EQ(std::any_cast<Position>(posAny), viewPos);
  }
}

TEST_F(ViewImplTest, MultiComponent) {
  ViewImpl<Position, bool> view({&posColumn, &boolColumn});
  EXPECT_EQ(std::distance(view.begin(), view.end()), 2);
  for (auto [entityId, pos, b] : view) {
    SCOPED_TRACE("entityId=" + std::to_string(entityId));
    switch (entityId) {
    case 0:
      EXPECT_EQ(pos, (Position{1, 2}));
      EXPECT_EQ(b, true);
      break;
    case 2:
      EXPECT_EQ(pos, (Position{5, 6}));
      EXPECT_EQ(b, false);
      break;
    default:
      FAIL() << "Unexpected entityId " << entityId;
    }
  }
}

TEST_F(ViewImplTest, EmptyIntersection) {
  // `intColumn` and `boolColumn` have disjoint sets of entities.
  ViewImpl<Position, int, bool> view({&posColumn, &intColumn, &boolColumn});
  EXPECT_EQ(view.begin(), view.end());
}

TEST_F(ViewImplTest, FilterViewedColumn) {
  ViewImpl<Position, bool> view({&posColumn, &boolColumn});
  // Filtering on any column which is viewed results in no values iterated.
  view.filterColumn(&boolColumn);
  EXPECT_EQ(view.begin(), view.end());
}

TEST_F(ViewImplTest, FilterDisjointColumn) {
  ViewImpl<Position, bool> view({&posColumn, &boolColumn});
  // Filtering on a column which is disjoint w.r.t. the viewed columns should
  // not impact the values iterated.
  view.filterColumn(&intColumn);
  EXPECT_EQ(std::distance(view.begin(), view.end()), 2);
}

TEST_F(ViewImplTest, FilterOverlappingColumn) {
  ViewImpl<Position, bool> view({&posColumn, &boolColumn});
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

TEST(ComponentsTest, EmptyInvariants) {
  Components components;
  EXPECT_EQ(components.has<Position>(0), false);
  EXPECT_EQ(components.get<Position>(0), nullptr);
}

TEST(ComponentsTest, Emplace) {
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
  components.emplace<int>(0, 42);

  components.emplace<Position>(1, 3, 4);

  // Test removal of a set of entities.
  ASSERT_TRUE(components.has<Position>(0));
  ASSERT_TRUE(TestComponentValue(components, 0, Position{1, 2}));
  ASSERT_TRUE(components.has<int>(0));
  ASSERT_TRUE(TestComponentValue(components, 0, 42));
  ASSERT_TRUE(components.has<Position>(1));
  ASSERT_TRUE(TestComponentValue(components, 1, Position{3, 4}));
  components.remove(0, std::vector{componentId<Position>, componentId<int>});
  EXPECT_FALSE(components.has<Position>(0));
  EXPECT_EQ(components.get<Position>(0), nullptr);
  EXPECT_FALSE(components.has<int>(0));
  EXPECT_EQ(components.get<int>(0), nullptr);
  ASSERT_TRUE(components.has<Position>(1));
  EXPECT_TRUE(TestComponentValue(components, 1, Position{3, 4}));
}

// We test Components::view() for both const and non-const overloads.
template <typename T>
class ComponentsViewTest : public testing::Test {

protected:
  void SetUp() override {
    components.emplace<Position>(0, 1, 2);
    components.emplace<Position>(1, 3, 4);
    components.emplace<Position>(2, 5, 6);
    components.emplace<Position>(3, 7, 8);
    components.set<int>(0, 42);
    components.set<int>(2, 123);
    components.set<int>(3, 456);
    components.set<bool>(0, true);
    components.set<bool>(2, true);
    components.set<float>(3, 3.14f);
  }

  Components components;
};
TYPED_TEST_SUITE_P(ComponentsViewTest);

TYPED_TEST_P(ComponentsViewTest, UnfilteredView) {
  // Create a view of the components.
  auto view = const_cast<TypeParam&>(this->components)
                  // Try adding a const qualifier to the viewed type.
                  .template view<Position, const int>();
  auto begin = view.begin();
  auto end = view.end();

  EXPECT_EQ(std::distance(begin, end), 3);

  for (auto [entityId, posVal, intVal] : view) {
    static_assert(std::is_same_v<decltype(entityId), EntityId>,
                  "entityId is not an EntityId");
    if constexpr (std::is_const_v<TypeParam>) {
      static_assert(std::is_same_v<decltype(posVal), const Position&>,
                    "posVal has the wrong type");
      static_assert(std::is_same_v<decltype(intVal), const int&>,
                    "intVal has the wrong type");
    } else {
      static_assert(std::is_same_v<decltype(posVal), Position&>,
                    "posVal has the wrong type");
      // Note the const-qualification here even though we are testing the
      // non-const overload of `view()`. This is because we requested a const-
      // qualified view over the `int` component.
      static_assert(std::is_same_v<decltype(intVal), const int&>,
                    "intVal has the wrong type");
    }

    SCOPED_TRACE("entityId=" + std::to_string(entityId));
    switch (entityId) {
    case 0:
      EXPECT_EQ(posVal, (Position{1, 2}));
      EXPECT_EQ(intVal, 42);
      break;
    case 2:
      EXPECT_EQ(posVal, (Position{5, 6}));
      EXPECT_EQ(intVal, 123);
      break;
    case 3:
      EXPECT_EQ(posVal, (Position{7, 8}));
      EXPECT_EQ(intVal, 456);
      break;
    default:
      FAIL() << "Unexpected entityId " << entityId;
    }
  }
}

TYPED_TEST_P(ComponentsViewTest, FilteredView) {
  // Create a view of the components filtered by just one column type.
  auto view = const_cast<TypeParam&>(this->components)
                  .template view<Position>(filter<bool>);
  auto begin = view.begin();
  auto end = view.end();

  EXPECT_EQ(std::distance(begin, end), 2);
  for (auto [entityId, posVal] : view) {
    SCOPED_TRACE("entityId=" + std::to_string(entityId));
    switch (entityId) {
    case 1:
      EXPECT_EQ(posVal, (Position{3, 4}));
      break;
    case 3:
      EXPECT_EQ(posVal, (Position{7, 8}));
      break;
    default:
      FAIL() << "Unexpected entityId " << entityId;
    }
  }

  // Create a view of the components filtered by two column types.
  view = const_cast<TypeParam&>(this->components)
             .template view<Position>(filter<bool, float>);
  begin = view.begin();
  end = view.end();
  EXPECT_EQ(std::distance(begin, end), 1);
  for (auto [entityId, posVal] : view) {
    SCOPED_TRACE("entityId=" + std::to_string(entityId));
    switch (entityId) {
    case 1:
      EXPECT_EQ(posVal, (Position{3, 4}));
      break;
    default:
      FAIL() << "Unexpected entityId " << entityId;
    }
  }
}

using ComponentsViewTypes = ::testing::Types<Components, const Components>;
REGISTER_TYPED_TEST_SUITE_P(ComponentsViewTest, UnfilteredView, FilteredView);
INSTANTIATE_TYPED_TEST_SUITE_P(ComponentsViewTests, ComponentsViewTest,
                               ComponentsViewTypes);

}  // namespace internal

// User-facing API tests.

TEST(RegistryTest, Entities) {
  // NOTE: Testing the removal of components in the destruction behavior is done
  // in the DestroyEntityWithComponents test below.

  Registry registry;

  EntityId entity = registry.createEntity();
  EXPECT_TRUE(registry.hasEntity(entity));
  EXPECT_TRUE(registry.destroyEntity(entity));
  EXPECT_FALSE(registry.hasEntity(entity));

  entity = registry.reserveEntity();
  EXPECT_FALSE(registry.hasEntity(entity));
  registry.instantiateEntity(entity);
  EXPECT_TRUE(registry.hasEntity(entity));
  EXPECT_TRUE(registry.destroyEntity(entity));
  EXPECT_FALSE(registry.hasEntity(entity));
}

TEST(RegistryTest, Components) {
  Registry registry;

  EntityId entity = registry.createEntity();
  registry.emplaceComponent<Position>(entity, 1, 2);
  registry.emplaceComponent<int>(entity, 42);
  registry.emplaceComponent<bool>(entity, true);

  ASSERT_TRUE(registry.hasComponent<Position>(entity));
  ASSERT_TRUE(registry.hasComponent<int>(entity));
  ASSERT_TRUE(registry.hasComponent<bool>(entity));

  EXPECT_EQ(*registry.getComponent<Position>(entity), (Position{1, 2}));
  EXPECT_EQ(*registry.getComponent<int>(entity), 42);
  EXPECT_EQ(*registry.getComponent<bool>(entity), true);
  {
    // Just double-check everything work as expected for const.
    auto const_registry = std::as_const(registry);
    EXPECT_EQ(*const_registry.getComponent<Position>(entity), (Position{1, 2}));
    EXPECT_EQ(*const_registry.getComponent<int>(entity), 42);
    EXPECT_EQ(*const_registry.getComponent<bool>(entity), true);
  }

  registry.removeComponent<Position>(entity);
  EXPECT_FALSE(registry.hasComponent<Position>(entity));
  EXPECT_TRUE(registry.hasComponent<int>(entity));
  EXPECT_TRUE(registry.hasComponent<bool>(entity));

  registry.removeComponent<int>(entity);
  EXPECT_FALSE(registry.hasComponent<int>(entity));
  EXPECT_TRUE(registry.hasComponent<bool>(entity));

  registry.removeComponent<bool>(entity);
  EXPECT_FALSE(registry.hasComponent<bool>(entity));
}

TEST(RegistryTest, DestroyEntityWithComponents) {
  Registry registry;

  EntityId entity = registry.createEntity();
  registry.emplaceComponent<Position>(entity, 1, 2);
  registry.emplaceComponent<int>(entity, 42);
  registry.emplaceComponent<bool>(entity, true);

  EXPECT_TRUE(registry.hasComponent<Position>(entity));
  EXPECT_TRUE(registry.hasComponent<int>(entity));
  EXPECT_TRUE(registry.hasComponent<bool>(entity));

  EXPECT_TRUE(registry.destroyEntity(entity));

  EXPECT_FALSE(registry.hasEntity(entity));
  EXPECT_FALSE(registry.hasComponent<Position>(entity));
  EXPECT_FALSE(registry.hasComponent<int>(entity));
  EXPECT_FALSE(registry.hasComponent<bool>(entity));
}

TEST(RegistryTest, View) {
  Registry registry;

  EntityId entity1 = registry.createEntity();
  registry.emplaceComponent<Position>(entity1, 1, 2);
  registry.emplaceComponent<int>(entity1, 42);
  registry.emplaceComponent<bool>(entity1, true);

  EntityId entity2 = registry.createEntity();
  registry.emplaceComponent<Position>(entity2, 3, 4);
  registry.emplaceComponent<int>(entity2, 43);
  registry.emplaceComponent<bool>(entity2, false);

  EntityId entity3 = registry.createEntity();
  registry.emplaceComponent<Position>(entity3, 5, 6);
  registry.emplaceComponent<int>(entity3, 44);
  registry.emplaceComponent<bool>(entity3, true);

  // Run checks in helper function to test const and non-const overloads.
  auto check = [entity1, entity2, entity3](auto& registry) {
    auto view = registry.template view<Position, int, bool>();
    auto begin = view.begin();
    auto end = view.end();
    EXPECT_EQ(std::distance(begin, end), 3);
    for (auto [entityId, posVal, intVal, boolVal] : view) {
      SCOPED_TRACE("entityId=" + std::to_string(entityId));
      if (entityId == entity1) {
        EXPECT_EQ(posVal, (Position{1, 2}));
        EXPECT_EQ(intVal, 42);
        EXPECT_EQ(boolVal, true);
      } else if (entityId == entity2) {
        EXPECT_EQ(posVal, (Position{3, 4}));
        EXPECT_EQ(intVal, 43);
        EXPECT_EQ(boolVal, false);
      } else if (entityId == entity3) {
        EXPECT_EQ(posVal, (Position{5, 6}));
        EXPECT_EQ(intVal, 44);
        EXPECT_EQ(boolVal, true);
      } else {
        FAIL() << "Unexpected entityId " << entityId;
      }
    }
  };
  check(registry);
  check(std::as_const(registry));
}

}  // namespace bad
