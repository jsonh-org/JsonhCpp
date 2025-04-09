#include "../jsonh_cpp/jsonh_cpp.h"
#include <gtest/gtest.h>

TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);

  long j = (new jsonh_reader(new std::string("")))->char_counter;
  EXPECT_EQ(j, j);
}