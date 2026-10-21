#pragma once

#include <atomic>
#include <cstdint>
#include <random>
#include <thread>

#include "shared_vector/SharedVector.hpp"

namespace concurrent {

class Updater {
public:
	Updater(SharedVector& vector, std::uint16_t frequency) noexcept;
	~Updater() noexcept;

	Updater(const Updater&) = delete;
	Updater& operator=(const Updater&) = delete;

	void start();
	void stop();

private:
	void run();

private:
	SharedVector& _vector;
	const std::uint16_t _frequency;
	std::atomic<bool> _is_running{false};
	std::thread _worker;
	std::mt19937 _random_engine{};
};

} // namespace concurrent
