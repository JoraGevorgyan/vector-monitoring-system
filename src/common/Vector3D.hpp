#pragma once

#include <ostream>

#include "common/types.hpp"

namespace common {

class Vector3D {
public:
	constexpr Vector3D() noexcept : _x{0.}, _y{0.}, _z{0.} {}

	constexpr Vector3D(el_t x, el_t y, el_t z) noexcept : _x{x}, _y{y}, _z{z} {}

	bool is_in_range(const Range& range) const noexcept;
	friend std::ostream& operator<<(std::ostream& os, const Vector3D& vector);

private:
	el_t _x;
	el_t _y;
	el_t _z;
};

} // namespace common
