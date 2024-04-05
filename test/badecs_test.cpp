#include <badecs/internal/Column.h>

#include <gtest/gtest.h>

namespace bad::internal {

TEST(ColumnTest, EmplaceTest) {
  Column column;
  ASSERT_EQ(column.size(), 0);
  EXPECT_EQ(column.has(0), false);
  column.emplace<int>(0, 1);
  EXPECT_EQ(column.has(0), true);
  EXPECT_EQ(column.size(), 1);
}

} // namespace bad::internal
