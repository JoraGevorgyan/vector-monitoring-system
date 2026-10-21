#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "shared_vector/SharedVector.hpp"
#include "updater/Updater.hpp"

TEST(UpdaterTest, UpdatesSharedVectorAndStops)
{
	concurrent::SharedVector& vector = concurrent::SharedVector::instance();
	vector.update(0., 0., 0.);

	concurrent::Updater updater(vector, 500);
	updater.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	updater.stop();

	EXPECT_TRUE(vector.get_snapshot().is_in_range({0, 174}));
}
