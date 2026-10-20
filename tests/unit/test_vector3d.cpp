#include <gtest/gtest.h>

#include "common/Vector3D.hpp"

TEST(Vector3DTest, DefaultIsZero)
{
	const common::Vector3D v;

	EXPECT_TRUE(v.is_in_range({0, 0}));
}

TEST(Vector3DTest, RangeIsInclusive)
{
	const common::Vector3D v(3., 4., 0.);

	EXPECT_TRUE(v.is_in_range({5, 5}));
	EXPECT_TRUE(v.is_in_range({0, 5}));
	EXPECT_TRUE(v.is_in_range({5, 10}));
}

TEST(Vector3DTest, OutsideRange)
{
	const common::Vector3D v(3., 4., 0.);

	EXPECT_FALSE(v.is_in_range({6, 10}));
	EXPECT_FALSE(v.is_in_range({0, 4}));
}
