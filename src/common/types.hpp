#pragma once

#include <cstdint>
#include <ostream>

namespace common {

using el_t = double;

struct Range {
	std::uint8_t left;
	std::uint8_t right;
};

inline std::ostream& operator<<(std::ostream& os, const Range& range)
{
	return os << "interval{L1=" << static_cast<unsigned int>(range.left)
						<< ", L2=" << static_cast<unsigned int>(range.right) << '}';
}

} // namespace common
