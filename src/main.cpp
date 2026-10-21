#include <array>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <thread>

#include "shared_vector/SharedVector.hpp"
#include "threshold_logger/ThresholdLogger.hpp"
#include "updater/Updater.hpp"

namespace {

constexpr std::uint16_t k_logger_count = 5;
constexpr std::uint16_t k_updater_count = 2;
constexpr std::uint16_t k_minimum_log_frequency = 1000;
constexpr std::uint16_t k_maximum_log_frequency = 10000;
constexpr std::uint16_t k_minimum_update_frequency = 10;
constexpr std::uint16_t k_maximum_update_frequency = 100;
constexpr std::uint16_t k_maximum_runtime_ms = 60000;

std::uint16_t random_frequency(
	std::mt19937& engine, std::uint16_t minimum, std::uint16_t maximum)
{
	return std::uniform_int_distribution<std::uint16_t>{minimum, maximum}(engine);
}

common::Range random_range(std::mt19937& engine)
{
	const auto left = std::uniform_int_distribution<unsigned int>{1, 180}(engine);
	const auto right =
		std::uniform_int_distribution<unsigned int>{left + 20, 200}(engine);
	return {static_cast<std::uint8_t>(left), static_cast<std::uint8_t>(right)};
}

std::chrono::milliseconds parse_runtime(int argc, char* argv[])
{
	if (argc == 1) {
		return std::chrono::milliseconds{k_maximum_runtime_ms};
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
		duration_ms > k_maximum_runtime_ms) {
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

		auto& vector = concurrent::SharedVector::instance();
		std::array<std::unique_ptr<concurrent::Updater>, k_updater_count> updaters;
		std::array<std::unique_ptr<concurrent::ThresholdLogger>, k_logger_count>
			loggers;

		for (auto& updater : updaters) {
			updater = std::make_unique<concurrent::Updater>(
				vector,
				random_frequency(
					engine, k_minimum_update_frequency, k_maximum_update_frequency));
		}
		for (auto& logger : loggers) {
			logger = std::make_unique<concurrent::ThresholdLogger>(
				vector,
				random_frequency(
					engine, k_minimum_log_frequency, k_maximum_log_frequency),
				random_range(engine));
		}

		for (auto& updater : updaters) {
			updater->start();
		}
		for (auto& logger : loggers) {
			logger->start();
		}

		std::this_thread::sleep_until(deadline);

		for (auto& updater : updaters) {
			updater->stop();
		}
		for (auto& logger : loggers) {
			logger->stop();
		}
	}
	catch (const std::system_error& err) {
		// std::mutex::lock may throw system_error
		std::cerr << err.what() << std::endl;
		return 1;
	}
	catch (const std::invalid_argument& err) {
		std::cerr << err.what() << std::endl;
		return 2;
	}

	return 0;
}
