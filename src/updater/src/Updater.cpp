#include "updater/Updater.hpp"

#include <chrono>

#include "common/helpers.hpp"

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
	if (_is_running.load(std::memory_order_relaxed)) {
		return;
	}

	if (_worker.joinable()) {
		_worker.join();
	}

	std::random_device random_device;
	_random_engine.seed(random_device());

	_is_running.store(true, std::memory_order_relaxed);
	_worker = std::thread(&Updater::run, this);
}

void Updater::stop()
{
	_is_running.store(false, std::memory_order_relaxed);
	if (_worker.joinable()) {
		_worker.join();
	}
}

void Updater::run()
{
	const auto sleep_period = common::get_sleep_period(_frequency);
	auto next_shot = std::chrono::steady_clock::now();

	while (_is_running.load(std::memory_order_relaxed)) {
		std::uniform_real_distribution<common::el_t> distribution(0.0, 100.0);
		_vector.update(
			distribution(_random_engine),
			distribution(_random_engine),
			distribution(_random_engine));

		next_shot += sleep_period;
		if (next_shot < std::chrono::steady_clock::now()) {
			next_shot = std::chrono::steady_clock::now();
		}
		std::this_thread::sleep_until(next_shot);
	}
}

} // namespace concurrent
