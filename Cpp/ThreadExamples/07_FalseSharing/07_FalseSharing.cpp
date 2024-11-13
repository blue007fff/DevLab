// 07_FalseSharing.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <functional>
#include <thread>
#include <vector>
#include <utility>
#include <format>

//cache-line 무효화로, 멀티스레드 성능이 싱글스레드 보다 낮아짐.
//- 보통 cache-line 이 64 byte 단위이기 때문에,
//- 64 byte 메모리 정렬로 해결 가능.
#define SHOW_FALSE_SHARING
#ifdef SHOW_FALSE_SHARING
#define MYALIGN
#else
#define MYALIGN alignas(64)
#endif // SHOW_FALSE_SHARING

//#define MYALIGN // false sharing
MYALIGN double mt_num1 = 0;
MYALIGN double mt_num2 = 0;
MYALIGN double mt_num3 = 0;
MYALIGN double mt_num4 = 0;
double st_num = 0;
constexpr int64_t default_iteration_count = 100'000'000;
constexpr double default_sum_value = 0.001;

class ScopedTimer {
public:
	using clock = std::chrono::steady_clock;
	using Func = std::function<void(double)>;

	ScopedTimer(Func&& func) : m_func(func) {}
	~ScopedTimer() {
		double elaspedTime = std::chrono::duration<double>(clock::now() - m_stp).count();
		m_func(elaspedTime);
	}
	clock::time_point m_stp{clock::now()};
	std::function<void(double)> m_func;
};

void work1(int64_t n)
{
	for (int64_t i = 0; i < n; i++)
		mt_num1 += default_sum_value;
}

void work2(int64_t n)
{
	for (int64_t i = 0; i < n; i++)
		mt_num2 += default_sum_value;
}

void work3(int64_t n)
{
	for (int64_t i = 0; i < n; i++)
		mt_num3 += default_sum_value;
}

void work4(int64_t n)
{
	for (int64_t i = 0; i < n; i++)
		mt_num4 += default_sum_value;
}

void MultiThreadWorkd(int64_t n)
{
	ScopedTimer timer([](double time) {
		std::cout << std::format("multi-thread: {}: {}\n",
		mt_num1 + mt_num2 + mt_num3 + mt_num4, time); });

	std::thread t1(work1, n / 4);
	std::thread t2(work2, n / 4);
	std::thread t3(work3, n / 4);
	std::thread t4(work4, n / 4);
	t1.join();
	t2.join();
	t3.join();
	t4.join();
}

void SingleThreadWork(int64_t n)
{
	ScopedTimer timer([](double time) {
		std::cout << std::format("single-thread: {}: {}\n",
		st_num, time); });

	for (int64_t i = 0; i < n; i++)
	{
		st_num += default_sum_value;
	}
}

int main()
{
	std::cout << "cache-line: " << std::hardware_constructive_interference_size << std::endl;
	auto* pNum1 = &mt_num1;
	auto* pNum2 = &mt_num2;
	auto byteDifference = reinterpret_cast<char*>(pNum2) - reinterpret_cast<char*>(pNum1);
	std::cout << "diff(mt_num2, mt_num1): " << byteDifference << std::endl;

	int n = default_iteration_count;
	std::cout << "iter: ";
	std::cout << n << std::endl;
	// std::cin >> n;

	SingleThreadWork(n);
	MultiThreadWorkd(n);
}