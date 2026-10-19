#include "common/Greet.hpp"

namespace common {
std::string greet(const std::string& name)
{
	return "Hello, " + name + "!";
}
} // namespace common
