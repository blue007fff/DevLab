#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <string>
#include <format>
#include <span>

namespace helpers
{
	template <typename T>
	void PrintRepeatedChar(T c, size_t count) requires std::same_as<T, char>
	{
		//std::cout << std::format("{:-<{}}\n", "", 50);
		std::cout << std::format("{}\n", std::string(count, c));
	}

	template <typename T>
	void PrintContainer(std::span<const T> container, std::string sep = ", ")
	{
		// for (T i : container)
		//     std::cout << i << sep;
		// std::cout << std::endl;

		// std::copy(container.begin(), container.end(),
		//           std::ostream_iterator<T>(std::cout, sep));
		// std::cout << '\n';

		/*std::copy(list.begin(), list.end(),
			std::ostream_iterator<T>(std::cout, ", "));
		std::cout << '\n';*/

		std::for_each(container.begin(), container.end(), [&sep](const T& v)
			{ std::cout << v << sep; });
		std::cout << '\n';
	}

	template <typename T>
	void PrintContainer(const T& container, std::string sep = ", ")
	{
		using value_type = typename T::value_type;
		PrintContainer(std::span<const value_type>(container.begin(), container.end()), sep);
	}
}
