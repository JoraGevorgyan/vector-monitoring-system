#include <gtest/gtest.h>

#include <stdexcept>

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
