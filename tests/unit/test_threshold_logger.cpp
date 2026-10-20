#include <gtest/gtest.h>

#include <stdexcept>

#include "shared_vector/SharedVector.hpp"
#include "threshold_logger/ThresholdLogger.hpp"

TEST(ThresholdLoggerTest, RejectsZeroFrequency)
{
	EXPECT_THROW(
		concurrent::ThresholdLogger(
			concurrent::SharedVector::instance(), 0, {0, 10}),
		std::invalid_argument);
}

TEST(ThresholdLoggerTest, RejectsInvertedRange)
{
	EXPECT_THROW(
		concurrent::ThresholdLogger(
			concurrent::SharedVector::instance(), 10, {10, 0}),
		std::invalid_argument);
}

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
