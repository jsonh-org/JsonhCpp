#include "../JsonhCpp/JsonhCpp.cpp"
#include <gtest/gtest.h>

TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);

  EXPECT_EQ(CalculateANumber(), CalculateANumber());
}