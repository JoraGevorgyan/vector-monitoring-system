#include "updater/Updater.hpp"

#include <chrono>

namespace concurrent {

Updater::Updater(SharedVector& vector, std::uint16_t frequency) noexcept
	: _vector{vector}, _frequency{frequency}
{
}

Updater::~Updater() noexcept
{
	stop();
}

void Updater::start()
{
	const std::lock_guard<std::mutex> lifecycle_lock(_lifecycle_mutex);
	if (_is_running.load(std::memory_order_acquire)) {
		return;
	}

	if (_worker.joinable()) {
		_worker.join();
	}

	std::random_device random_device;
	_random_engine.seed(random_device());

	_is_running.store(true, std::memory_order_release);
	try {
		_worker = std::thread(&Updater::run, this);
	}
	catch (...) {
		_is_running.store(false, std::memory_order_release);
		throw;
	}
}

void Updater::stop()
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

void Updater::run()
{
	const auto interval =
		std::chrono::duration_cast<std::chrono::steady_clock::duration>(
			std::chrono::duration<double>(1.0 / _frequency));
	std::uniform_real_distribution<common::el_t> distribution(0.0, 100.0);
	auto next_tick = std::chrono::steady_clock::now();

	while (_is_running.load(std::memory_order_acquire)) {
		_vector.update(
			distribution(_random_engine),
			distribution(_random_engine),
			distribution(_random_engine));

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

} // namespace concurrent
