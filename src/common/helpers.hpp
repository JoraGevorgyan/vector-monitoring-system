#pragma once

#include <chrono>

namespace common {

inline auto get_sleep_period(uint16_t frequency)
{
	return std::chrono::duration_cast<std::chrono::steady_clock::duration>(
		std::chrono::duration<double>(1.0 / frequency));
}

} // namespace common
