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

#include <mutex>
void Example2_Mutex()
{
	std::cout << __func__ << std::endl;

	std::mutex m;
	auto func1 = [&m](int& count) {
		std::thread::id tid = std::this_thread::get_id();
		std::cout << std::format("thread: {}: func(int& count)\n",
			reinterpret_cast<uint64_t>(std::addressof(tid)));

		for (auto i : std::ranges::iota_view{ 0, 100000 }) {
			std::unique_lock<std::mutex> lock(m);
			//m.lock();
			count++;
			//m.unlock();
		}
	};

	int count = 0;
	std::thread t1(func1, std::ref(count));
	std::thread t2(func1, std::ref(count));

	t1.join();
	t2.join();

	// result : 200000
	std::cout << std::format("count : {}\n", count);
}

#include <vector>
#include <condition_variable>
void Example_ConditionVariable()
{
	std::cout << __func__ << std::endl;

	std::mutex m;
	std::condition_variable cv;
	bool ready{ false };

	std::vector<std::thread> threads;

	auto func1 = [&m, &cv, &ready](int id) {
		std::cout << std::format("start thread: {}\n", id);

		std::unique_lock lk(m);
		//cv.wait(lk);
		cv.wait(lk, [&]() {return ready; });

		std::cout << std::format("end thread: {}\n", id);
	};

	for (int i = 0; i < 5; ++i)
		threads.emplace_back(func1, i);

	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	ready = true;

	for (auto& thd : threads) {
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		cv.notify_one();
	}
	cv.notify_all();

	for (auto& thd : threads) thd.join();
}

int main()
{
	auto PrintSplitLines = []() {std::cout << std::format("{:-<{}}\n", "", 30);};

	PrintSplitLines();
	Example1();

	PrintSplitLines();
	Example2();

	PrintSplitLines();
	Example2_Mutex();

	PrintSplitLines();
	Example_ConditionVariable();
}