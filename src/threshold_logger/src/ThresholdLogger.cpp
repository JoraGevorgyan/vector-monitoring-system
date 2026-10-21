#include "threshold_logger/ThresholdLogger.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <stdexcept>

namespace concurrent {

std::mutex ThresholdLogger::_out_mutex;

ThresholdLogger::ThresholdLogger(
	const SharedVector& vector, std::uint16_t frequency, common::Range range)
	: _vector{vector}, _range{range}, _frequency{frequency}
{
}

ThresholdLogger::~ThresholdLogger()
{
	stop();
}

void ThresholdLogger::start()
{
	if (!_is_running.load(std::memory_order_relaxed)) {
		_is_running.store(true, std::memory_order_relaxed);
		_runner = std::thread(&ThresholdLogger::run, this);
	}
}

void ThresholdLogger::stop()
{
	if (_runner.joinable()) {
		_is_running.store(false, std::memory_order_relaxed);
		_runner.join();
	}
}

void ThresholdLogger::run()
{
	const auto sleep_period = std::chrono::milliseconds(1000'000 / _frequency);
	auto next_shot = std::chrono::steady_clock::now();

	while (_is_running.load(std::memory_order_relaxed)) {
		const auto snapshot = _vector.get_snapshot();
		if (snapshot.is_in_range(_range)) {
			print_stdout(snapshot);
		}
		next_shot += sleep_period;
		if (next_shot < std::chrono::steady_clock::now()) {
			next_shot = std::chrono::steady_clock::now();
		}
		std::this_thread::sleep_until(next_shot);
	}
}

void ThresholdLogger::print_stdout(const common::Vector3D& snapshot) const
{
	const std::lock_guard<std::mutex> output_lock(_out_mutex);
	std::cout << _range << " : " << snapshot << "\n";
}

} // namespace concurrent
