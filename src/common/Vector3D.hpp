#pragma once

namespace common {

using el_t = double;

class Vector3D {
public:
	Vector3D() noexcept = default;
	Vector3D(el_t x, el_t y, el_t z) noexcept;

	bool is_in_range(el_t l_1, el_t l_2) const noexcept;

private:
	el_t _x;
	el_t _y;
	el_t _z;
};

} // namespace common
