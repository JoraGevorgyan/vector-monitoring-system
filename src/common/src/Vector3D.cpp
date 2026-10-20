#include "common/Vector3D.hpp"

namespace common {

bool Vector3D::is_in_range(const Range& range) const noexcept
{
	const el_t m2 = _x * _x + _y * _y + _z * _z;
	return (range.left * range.left <= m2) && (m2 <= range.right * range.right);
}

} // namespace common
