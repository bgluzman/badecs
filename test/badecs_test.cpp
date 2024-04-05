#include <badecs/internal/Column.h>

#include <gtest/gtest.h>

#include <string>

namespace bad::internal {

TEST(ColumnTest, EmptyInvariants) {
  Column column;
  EXPECT_EQ(column.size(), 0);
  EXPECT_EQ(column.has(0), false);
}

TEST(ColumnTest, EmplaceComponent) {
  Column column;
  ASSERT_EQ(column.size(), 0);
  ASSERT_EQ(column.has(0), false);
  column.emplace<int>(0, 1);
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
  // TODO (bgluzman): have to call get???
}

TEST(ColumnTest, SetComponent) {
  Column column;
  ASSERT_EQ(column.size(), 0);
  ASSERT_EQ(column.has(0), false);
  column.set(0, std::string{"hello world"});
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
  // TODO (bgluzman): have to call get???
  column.set(0, std::string{"goodbye world"});
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
  // TODO (bgluzman): have to call get???
}

} // namespace bad::internal
