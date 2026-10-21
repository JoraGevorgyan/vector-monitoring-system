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

TEST(UpdaterTest, WritesNonZeroValuesWithinComponentBounds)
{
	concurrent::SharedVector& vector = concurrent::SharedVector::instance();
	vector.update(0., 0., 0.);

	concurrent::Updater updater(vector, 1000);
	updater.start();

	// A zero vector is outside {1, 174}; wait until the updater replaces it.
	const auto deadline =
		std::chrono::steady_clock::now() + std::chrono::seconds(2);
	bool updated = false;
	while (std::chrono::steady_clock::now() < deadline) {
		if (vector.get_snapshot().is_in_range({1, 174})) {
			updated = true;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	updater.stop();

	EXPECT_TRUE(updated);
}

TEST(UpdaterTest, StartIsIdempotentAndCanRestart)
{
	concurrent::SharedVector& vector = concurrent::SharedVector::instance();

	concurrent::Updater updater(vector, 100);
	updater.start();
	updater.start();
	updater.stop();
	updater.stop();
	updater.start();
}
