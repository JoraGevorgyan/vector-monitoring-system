#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

#include "shared_vector/SharedVector.hpp"

namespace concurrent {

class ThresholdLogger {
public:
	ThresholdLogger(
		const SharedVector& vector, std::uint16_t frequency, common::Range range);
	~ThresholdLogger();

	ThresholdLogger(const ThresholdLogger&) = delete;
	ThresholdLogger& operator=(const ThresholdLogger&) = delete;

	void start();
	void stop();

private:
	void run();
	void print_stdout(const common::Vector3D& snapshot) const;

private:
	const SharedVector& _vector;
	const common::Range _range;
	const std::uint16_t _frequency;
	std::atomic<bool> _is_running{false};
	std::thread _worker;

	static std::mutex _out_mutex;
};

} // namespace concurrent
