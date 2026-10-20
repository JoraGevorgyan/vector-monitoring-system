#include "threshold_logger/ThresholdLogger.hpp"

#include <chrono>
#include <iostream>
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
	const std::lock_guard<std::mutex> lifecycle_lock(_lifecycle_mutex);
	if (_is_running.load(std::memory_order_acquire)) {
		return;
	}

	if (_worker.joinable()) {
		_worker.join();
	}

	_is_running.store(true, std::memory_order_release);
	try {
		_worker = std::thread(&ThresholdLogger::run, this);
	}
	catch (...) {
		_is_running.store(false, std::memory_order_release);
		throw;
	}
}

void ThresholdLogger::stop()
{
	const std::lock_guard<std::mutex> lifecycle_lock(_lifecycle_mutex);
	{
		const std::lock_guard<std::mutex> wait_lock(_wait_mutex);
		_is_running.store(false, std::memory_order_release);
	}
	_wait_condition.notify_all();

	if (_worker.joinable()) {
		_worker.join();
	}
}

void ThresholdLogger::run()
{
	const auto interval =
		std::chrono::duration_cast<std::chrono::steady_clock::duration>(
			std::chrono::duration<double>(1.0 / _frequency));
	auto next_tick = std::chrono::steady_clock::now();

	while (_is_running.load(std::memory_order_acquire)) {
		const auto snapshot = _vector.get_snapshot();
		if (snapshot.is_in_range(_range)) {
			print_stdout(snapshot);
		}

		next_tick += interval;
		std::unique_lock<std::mutex> wait_lock(_wait_mutex);
		if (_wait_condition.wait_until(wait_lock, next_tick, [this] {
					return !_is_running.load(std::memory_order_acquire);
				})) {
			break;
		}

		if (next_tick < std::chrono::steady_clock::now()) {
			next_tick = std::chrono::steady_clock::now();
		}
	}
}

void ThresholdLogger::print_stdout(const common::Vector3D& snapshot) const
{
	const std::lock_guard<std::mutex> output_lock(_out_mutex);
	std::cout << _range << " : " << snapshot << "\n";
}

} // namespace concurrent
