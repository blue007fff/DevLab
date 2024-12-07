#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <format>
#include <span>
#include <ctime>
#include <time.h>
#include <chrono>

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

		auto it = container.begin();
		std::cout << *it++;
		std::for_each(it, container.end(), [&sep](const T& v)
			{ std::cout << sep << v; });
		std::cout << '\n';
	}

	template <typename T>
	void PrintContainer(const T& container, std::string sep = ", ")
	{
		using value_type = typename T::value_type;
		PrintContainer(std::span<const value_type>(container.begin(), container.end()), sep);
	}

	template <typename T = void>
	std::tm GetLocaleTime()
	{
		auto now = std::chrono::system_clock::now();
		auto now_time_t = std::chrono::system_clock::to_time_t(now);
		std::tm localTime;
		localtime_s(&localTime, &now_time_t);

		/*std::ostringstream oss;
		oss << std::put_time(&localTime, "%c");
		return oss.str();*/
		return localTime;
	}
}
