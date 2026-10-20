#include "common/Vector3D.hpp"

namespace common {

bool Vector3D::is_in_range(const Range& range) const noexcept
{
	const el_t m2 = _x * _x + _y * _y + _z * _z;
	return (range.left * range.left <= m2) && (m2 <= range.right * range.right);
}

std::ostream& operator<<(std::ostream& os, const Vector3D& vector)
{
	return os << "{x=" << vector._x << ", y=" << vector._y << ", z=" << vector._z
						<< '}';
}

} // namespace common
