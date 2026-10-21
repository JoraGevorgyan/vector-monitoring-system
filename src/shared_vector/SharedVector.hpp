#pragma once

#include <shared_mutex>

#include "common/Vector3D.hpp"

namespace concurrent {

class SharedVector {
public:
	static SharedVector& instance() noexcept;

	common::Vector3D get_snapshot() const noexcept;
	void update(common::el_t x, common::el_t y, common::el_t z);

	SharedVector(const SharedVector&) = delete;
	SharedVector& operator=(const SharedVector&) = delete;
	SharedVector(SharedVector&&) = delete;
	SharedVector& operator=(SharedVector&&) = delete;

private:
	SharedVector() noexcept = default;
	~SharedVector() = default;

	common::Vector3D _vector;
	mutable std::shared_mutex _sh_mutex;
};

} // namespace concurrent
