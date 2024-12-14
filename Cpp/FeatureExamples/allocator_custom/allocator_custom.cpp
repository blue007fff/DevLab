#include <iostream>
#include <random>
#include <format>
#include <iomanip>
#include <map>
#include <windows.h>
#include <locale>
#include <iostream>
#include <memory>
#include <string>
#include <list>

#include "../../helpers.h"

namespace custom1
{
	template <typename T>
	struct allocator
	{
		using value_type = T; // value_type 필수
#if 0
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
#endif	

#if 0   // 17 에서 deprecated
		template <class U>
		struct rebind { typedef allocator<U> other; };
#endif

		allocator() noexcept = default;

		// 다른 타입과의 변환을 지원해 주어야 함.
		template <typename U>
		allocator(const allocator<U>&) noexcept {}

		T* allocate(std::size_t n) {
			std::cout << __FUNCTION__ << ": " << n << std::endl;
			void* ptr = ::operator new(sizeof(T) * n, std::align_val_t(alignof(T)));
			if (!ptr) { throw std::bad_alloc(); }
			return static_cast<T*>(ptr);
		}

		void deallocate(T* ptr, std::size_t n) noexcept {
			std::cout << __FUNCTION__ << ": " << n << std::endl;
			::operator delete(ptr, std::align_val_t(alignof(T)));
		}
	};

	void test()
	{
		helpers::PrintRepeatedChar('-', 50);
		std::cout << __FUNCTION__ << std::endl;

		helpers::PrintRepeatedChar('-', 30);
		{
			std::vector<int, custom1::allocator<int>> vec;

			vec.push_back(10);
			vec.push_back(20);
			vec.push_back(30);

			std::cout << "Vector contents: ";
			for (const auto& elem : vec) {
				std::cout << elem << " ";
			}
			std::cout << std::endl;
		}

		helpers::PrintRepeatedChar('-', 30);
		{
			std::list<int, custom1::allocator<int>> list;

			list.push_back(10);
			list.push_back(20);
			list.push_back(30);

			std::cout << "List contents: ";
			for (const auto& elem : list) {
				std::cout << elem << " ";
			}
			std::cout << std::endl;
		}
	}
}

int main()
{
	custom1::test();

	return 0;
}