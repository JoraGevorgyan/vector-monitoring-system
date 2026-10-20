#include "common/Vector3D.hpp"

namespace common {

bool Vector3D::is_in_range(el_t l_1, el_t l_2) const noexcept
{
	const el_t m2 = _x * _x + _y * _y + _z * _z;
	return (l_1 * l_1 <= m2) && (m2 <= l_2 * l_2);
}

} // namespace common
