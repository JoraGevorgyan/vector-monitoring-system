#pragma once

namespace common {

using el_t = double;

class Vector3D {
public:
	constexpr Vector3D() noexcept : _x{0.}, _y{0.}, _z{0.} {}

	constexpr Vector3D(el_t x, el_t y, el_t z) noexcept : _x{x}, _y{y}, _z{z} {}

	bool is_in_range(el_t l_1, el_t l_2) const noexcept;

private:
	el_t _x;
	el_t _y;
	el_t _z;
};

} // namespace common
