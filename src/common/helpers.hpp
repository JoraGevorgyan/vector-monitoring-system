#pragma once

#include <chrono>
#include <cstdint>

namespace common {

inline auto get_sleep_period(std::uint16_t frequency)
{
	return std::chrono::duration_cast<std::chrono::steady_clock::duration>(
		std::chrono::duration<double>(1.0 / frequency));
}

} // namespace common
