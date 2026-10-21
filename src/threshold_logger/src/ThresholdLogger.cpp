#include "threshold_logger/ThresholdLogger.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

#include "common/helpers.hpp"

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
		_worker = std::thread(&ThresholdLogger::run, this);
	}
}

void ThresholdLogger::stop()
{
	_is_running.store(false, std::memory_order_relaxed);
	if (_worker.joinable()) {
		_worker.join();
	}
}

void ThresholdLogger::run()
{
	const auto sleep_period = common::get_sleep_period(_frequency);
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
	std::ostringstream line;
	line << _range << " : " << snapshot << '\n';
	const std::string text = line.str();

	const std::lock_guard<std::mutex> output_lock(_out_mutex);
	std::cout.write(text.data(), static_cast<std::streamsize>(text.size()));
}

} // namespace concurrent
