#include <gtest/gtest.h>

#include <stdexcept>
#include <thread>

#include "shared_vector/SharedVector.hpp"
#include "threshold_logger/ThresholdLogger.hpp"

TEST(ThresholdLoggerTest, StartIsIdempotentAndCanRestart)
{
	concurrent::SharedVector& vector = concurrent::SharedVector::instance();
	vector.update(100., 0., 0.);

	{
		concurrent::ThresholdLogger logger(vector, 10, {0, 10});
		logger.start();
		logger.start();
		logger.stop();
		logger.stop();
		logger.start();
	}

	{
		concurrent::ThresholdLogger logger(vector, 10, {0, 10});
		logger.start();
	}
}

TEST(ThresholdLoggerTest, RunsWhileSharedVectorIsUpdatedConcurrently)
{
	concurrent::SharedVector& vector = concurrent::SharedVector::instance();
	vector.update(0., 0., 0.);

	concurrent::ThresholdLogger logger(vector, 1000, {255, 255});
	logger.start();

	std::thread updater([&vector] {
		for (int i = 0; i < 1000; ++i) {
			vector.update(i, i, i);
		}
		vector.update(3., 4., 0.);
	});

	updater.join();
	logger.stop();

	EXPECT_TRUE(vector.get_snapshot().is_in_range({5, 5}));
}
