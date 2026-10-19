#include <gtest/gtest.h>

#include "common/Greet.hpp"

TEST(GreetTest, ReturnsGreeting)
{
	EXPECT_EQ(common::greet("Alice"), "Hello, Alice!");
}
