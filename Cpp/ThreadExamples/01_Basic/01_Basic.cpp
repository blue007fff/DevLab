// 01_Basic.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <thread>
#include <format>
#include <string>
#include <ranges>

void Example1()
{
	std::cout << __func__ << std::endl;

	auto func1 = []() {
		std::thread::id tid = std::this_thread::get_id();
		std::cout << std::format("thread : {}: func1()\n", 
			reinterpret_cast<uint64_t>(std::addressof(tid)));
	};

	auto func2 = [](int a, int b) {
		std::thread::id tid = std::this_thread::get_id();
		std::cout << std::format("thread: {}: func({}, {})\n",
			reinterpret_cast<uint64_t>(std::addressof(tid)), a, b);
	};
	
	std::thread::id tid = std::this_thread::get_id();
	std::cout << std::format("main-thread : {}\n",
		reinterpret_cast<uint64_t>(std::addressof(tid)));

	std::thread t1(func1);
	std::thread t2(func2, 1, 2);
	t1.join();
	t2.join();
}

void Example2()
{
	std::cout << __func__ << std::endl;

	auto func1 = [](int& count) {
		std::thread::id tid = std::this_thread::get_id();
		std::cout << std::format("thread: {}: func(int& count)\n",
			reinterpret_cast<uint64_t>(std::addressof(tid)));
		for (auto i : std::ranges::iota_view{ 0, 100000 }) {
			count++;
		}		
	};

	int count = 0;
	std::thread t1(func1, std::ref(count));
	std::thread t2(func1, std::ref(count));

	t1.join();
	t2.join();

	// result : not 200000
	std::cout << std::format("count : {}\n", count);
}

int main()
{
	auto PrintSplitLines = []() {std::cout << std::format("{:-<{}}\n", "", 30);};

	PrintSplitLines();
	Example1();

	PrintSplitLines();
	Example2();
}