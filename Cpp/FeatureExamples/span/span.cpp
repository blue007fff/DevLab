#include <iostream>
#include <vector>
#include <span>

template<typename T>
void print(std::span<T> s)
{
	for (const auto& value : s)
		std::cout << value << " ";
	std::cout << std::endl;
}

template<typename T>
void printConst(std::span<const T> s)
{
	//s[0] = 10; // error
	for (const auto& value : s)
		std::cout << value << " ";
	std::cout << std::endl;
}

template<typename T>
void printReverse(std::span<T> s)
{
	for (auto it = s.rbegin(); it != s.rend(); ++it)
		std::cout << *it << " ";
	std::cout << std::endl;
}

int main()
{
	// span member
	// using pointer = T*;
	// pointer _Mydata{ nullptr };
	// size_t _Mysize{0};
	std::cout << "sizeof(span<int>): " << sizeof(std::span<int>) << std::endl;

	std::vector<int> v0{1, 2, 3, 4, 5};
	const std::vector<int> v1{1, 2, 3, 4, 5};

	std::span<int> s0 = v0;
	std::span<int> s1(v0.begin(), v0.end());
	std::span<int> s2(v0.begin(), v0.begin() + 3);
	//std::span<int> s2c = v1; //compile-error

	std::span<const int> s3(v0.begin(), 4);
	std::span<const int> s3c(v1.begin(), 4);

	print(s0);
	print(s1);
	print(s2);
	print(s3);
	//printConst(s2); //compile-error
	printConst(s3);
	printReverse(s2);
}