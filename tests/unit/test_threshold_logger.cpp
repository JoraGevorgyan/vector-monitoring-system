#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
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

namespace {

// Captures std::cout for the lifetime of the object.
class CoutCapture {
public:
	CoutCapture() : _previous{std::cout.rdbuf(_buffer.rdbuf())} {}

	~CoutCapture() { std::cout.rdbuf(_previous); }

	CoutCapture(const CoutCapture&) = delete;
	CoutCapture& operator=(const CoutCapture&) = delete;

	std::string str() const { return _buffer.str(); }

private:
	std::ostringstream _buffer;
	std::streambuf* _previous;
};

std::string run_logger(common::Range range)
{
	concurrent::SharedVector& vector = concurrent::SharedVector::instance();
	vector.update(3., 4., 0.);

	const CoutCapture capture;
	{
		concurrent::ThresholdLogger logger(vector, 1000, range);
		logger.start();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		logger.stop();
	}
	return capture.str();
}

} // namespace

TEST(ThresholdLoggerTest, PrintsVectorOnInclusiveBoundary)
{
	const std::string output = run_logger({5, 5});

	ASSERT_FALSE(output.empty());
	std::istringstream lines(output);
	std::string line;
	while (std::getline(lines, line)) {
		EXPECT_EQ(line, "interval{L1=5, L2=5} : {x=3, y=4, z=0}");
	}
}

TEST(ThresholdLoggerTest, PrintsNothingOutsideRange)
{
	EXPECT_TRUE(run_logger({6, 10}).empty());
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
