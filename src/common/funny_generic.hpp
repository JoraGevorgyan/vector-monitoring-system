#pragma once

#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

namespace funny {

template <typename T, typename = void>
struct is_iterable : std::false_type {};

template <typename T>
struct is_iterable<
	T,
	std::void_t<
		decltype(std::begin(std::declval<T&>())),
		decltype(std::end(std::declval<T&>()))>> : std::true_type {};

template <typename cont_t, typename mem_fun_t>
constexpr bool has_callable_member_v = [] {
	if constexpr (is_iterable<cont_t>::value) {
		using Element = decltype(*std::begin(std::declval<cont_t&>()));

		return std::is_invocable_v<mem_fun_t, Element>;
	}
	return false;
}();

template <
	typename cont_t,
	typename mem_fun_t,
	std::enable_if_t<has_callable_member_v<cont_t, mem_fun_t>, int> = 0>
void for_each_member(cont_t&& container, mem_fun_t member)
{
	for (auto&& element : container) {
		std::invoke(member, element);
	}
}

} // namespace funny
