#include <iostream>
#include <thread>
#include <atomic>
#include <format>
#include <string>

namespace test1
{
	int x = 0;
	int y = 0;
#define VOLATILE //volatile
	//volatile 는 최적화 방지.
	//메모리 접근 순서는 막지 못함.
	VOLATILE int r1 = 0;
	VOLATILE int r2 = 0;
	volatile bool ready;

	void Func1()
	{
		while (!ready);
		y = 1; // 쓰기
		r1 = x; // 읽기
	}
	void Func2()
	{
		while (!ready);
		x = 1; // 쓰기
		r2 = y; // 읽기
	}
	void Run()
	{
		int count = 0;
		while (true)
		{
			count++;
			x = y = 0;
			r1 = r2 = 0;
			ready = false;

			std::thread t1(Func1);
			std::thread t2(Func2);

			ready = true;

			t1.join();
			t2.join();

			// Check if reordering caused both loads to see 0
			if (r1 == 0 && r2 == 0) {
				break;
			}
		}
		std::cout << std::format("test1: count: {}\n", count);
	}
}

namespace test2
{
	std::atomic<int> x{ 0 };
	std::atomic<int> y{ 0 };
	int r1 = 0, r2 = 0;
	volatile bool ready{};

	void Func1() {
		while (!ready);
		x.store(1, std::memory_order_relaxed); // x = 1
		std::this_thread::yield(); // Hint for scheduler to increase chance of interleaving
		r1 = y.load(std::memory_order_relaxed); // r1 = y
	}
	void Func2() {
		while (!ready);
		y.store(1, std::memory_order_relaxed); // y = 1
		std::this_thread::yield(); // Hint for scheduler to increase chance of interleaving
		r2 = x.load(std::memory_order_relaxed); // r2 = x
	}
	void Run()
	{
		int count = 0;
		while (true)
		{
			count++;
			x = y = 0;
			r1 = r2 = 0;
			ready = false;

			std::thread t1(Func1);
			std::thread t2(Func2);

			ready = true;

			t1.join();
			t2.join();

			// Check if reordering caused both loads to see 0
			if (r1 == 0 && r2 == 0) {
				break;
			}
		}
		std::cout << std::format("test2: count: {}\n", count);
	}
}

namespace test3
{
	volatile bool ready{};

	void Run()
	{
		auto Producer = [](std::atomic<bool>* is_ready, int* data)
		{
			while (!ready);
			*data = 10;
			is_ready->store(true, std::memory_order_relaxed);
			//is_ready->store(true, std::memory_order_release);
			//해당 명령 이전의 모든 메모리 명령들이 해당 명령 이후로 재배치 되는 것을 금지
		};
		auto Consumer = [](std::atomic<bool>* is_ready, int* data)
		{
			while (!ready);
			// data 가 준비될 때 까지 기다린다.
			while (!is_ready->load(std::memory_order_relaxed)) {}
			//while (!is_ready->load(std::memory_order_acquire)) {}
			//해당 명령 뒤에 오는 모든 메모리 명령들이 해당 명령 위로 재배치 되는 것을 금지 합니다.
		};
		int count = 0;
		while (true)
		{
			count++;
			std::atomic<bool> is_ready(false);
			int data = 0;
			ready = false;

			std::thread t2(Consumer, &is_ready, &data);
			std::thread t1(Producer, &is_ready, &data);
			ready = true;

			t1.join();
			t2.join();

			if (data == 0)
				break;
		}
		std::cout << std::format("test3: count: {}\n", count);
	}
}

int main()
{
	// test1 은 자주 발생.
	// test2, test3 번은 확인 하기 어려움.

	std::thread t1(test1::Run);
	std::thread t2(test2::Run);
	std::thread t3(test3::Run);

	t1.join(); t2.join(); t3.join();
	return 0;
}