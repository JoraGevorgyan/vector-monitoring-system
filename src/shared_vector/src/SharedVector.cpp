#include "shared_vector/SharedVector.hpp"

#include <mutex>

#include "common/Vector3D.hpp"

namespace concurrent {

SharedVector& SharedVector::instance() noexcept
{
	static SharedVector inst;
	return inst;
}

common::Vector3D SharedVector::get_snapshot() const noexcept
{
	const std::lock_guard<std::mutex> l(_m);
	return _vector;
}

void SharedVector::update(common::el_t x, common::el_t y, common::el_t z)
{
	const std::lock_guard<std::mutex> l(_m);
	_vector = common::Vector3D(x, y, z);
}

} // namespace concurrent
