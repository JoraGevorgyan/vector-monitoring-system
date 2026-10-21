#include <array>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <thread>

#include "common/funny_generic.hpp"
#include "shared_vector/SharedVector.hpp"
#include "threshold_logger/ThresholdLogger.hpp"
#include "updater/Updater.hpp"

namespace {

constexpr std::uint16_t g_logger_count = 5;
constexpr std::uint16_t g_updater_count = 2;
constexpr std::uint16_t g_minimum_log_frequency = 1000;
constexpr std::uint16_t g_maximum_log_frequency = 10'000;
constexpr std::uint16_t g_minimum_update_frequency = 10;
constexpr std::uint16_t g_maximum_update_frequency = 100;
constexpr std::uint16_t g_maximum_runtime_ms = 60'000;

std::uint16_t random_frequency(
	std::mt19937& engine, std::uint16_t min, std::uint16_t max)
{
	return std::uniform_int_distribution<std::uint16_t>{min, max}(engine);
}

common::Range random_range(std::mt19937& engine)
{
	const auto left = std::uniform_int_distribution<int>{1, 179}(engine);
	const auto right = std::uniform_int_distribution<int>{left + 20, 200}(engine);
	return {static_cast<std::uint8_t>(left), static_cast<std::uint8_t>(right)};
}

std::chrono::milliseconds parse_runtime(int argc, char* argv[])
{
	if (argc == 1) {
		return std::chrono::milliseconds{g_maximum_runtime_ms};
	}

	if (argc != 3 || std::string_view{argv[1]} != "--duration-ms") {
		throw std::invalid_argument{
			"usage: vector_monitoring [--duration-ms 1..60000]"};
	}

	unsigned int duration_ms = 0;
	const std::string_view argument{argv[2]};
	const auto result = std::from_chars(
		argument.data(), argument.data() + argument.size(), duration_ms);
	if (
		result.ec != std::errc{} ||
		result.ptr != argument.data() + argument.size() || duration_ms == 0 ||
		duration_ms > g_maximum_runtime_ms) {
		throw std::invalid_argument{
			"duration must be an integer from 1 to 60000 ms"};
	}

	return std::chrono::milliseconds{duration_ms};
}

} // namespace

int main(int argc, char* argv[])
{
	try {
		const auto runtime = parse_runtime(argc, argv);
		const auto deadline = std::chrono::steady_clock::now() + runtime;
		std::random_device random_device;
		std::mt19937 engine{random_device()};

		auto& sh_vector = concurrent::SharedVector::instance();
		std::array<std::unique_ptr<concurrent::Updater>, g_updater_count> updaters;
		std::array<std::unique_ptr<concurrent::ThresholdLogger>, g_logger_count>
			loggers;

		for (auto& updater : updaters) {
			updater = std::make_unique<concurrent::Updater>(
				sh_vector,
				random_frequency(
					engine, g_minimum_update_frequency, g_maximum_update_frequency));
		}
		for (auto& logger : loggers) {
			logger = std::make_unique<concurrent::ThresholdLogger>(
				sh_vector,
				random_frequency(
					engine, g_minimum_log_frequency, g_maximum_log_frequency),
				random_range(engine));
		}

		funny::for_each_member(updaters, &concurrent::Updater::start);
		funny::for_each_member(loggers, &concurrent::ThresholdLogger::start);

		std::this_thread::sleep_until(deadline);

		funny::for_each_member(updaters, &concurrent::Updater::stop);
		funny::for_each_member(loggers, &concurrent::ThresholdLogger::stop);
	}
	catch (const std::system_error& err) {
		std::cerr << err.what() << std::endl;
		return 1;
	}
	catch (const std::invalid_argument& err) {
		std::cerr << err.what() << std::endl;
		return 2;
	}
	catch (const std::exception& err) {
		std::cerr << "Unexpected error: " << err.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "Unknown error occurred." << std::endl;
		return 1;
	}

	return 0;
}
