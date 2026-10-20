#include <iostream>
#include <system_error>

#include "shared_vector/SharedVector.hpp"

// void Vector3D::rand_init() noexcept
// {
// 	thread_local std::mt19937_64 gen{std::random_device{}()};
// 	std::uniform_real_distribution<el_t> dist{0.0, k_max};
// 	_x = dist(gen);
// 	_y = dist(gen);
// 	_z = dist(gen);
// }

int main()
{
	try {
		concurrent::SharedVector::instance().update(0.0, 0.0, 0.0);
	}
	catch (const std::system_error& err) {
		// std::mutex::lock may throw system_error
		std::cerr << err.what() << std::endl;
	}

	return 0;
}
