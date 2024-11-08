#include <iostream>
#include <thread>
#include <atomic>
#include <format>
#include <string>
#include <future>

//https://modoocode.com/284

namespace
{
	auto PrintSplitLines = []() {std::cout << std::format("{:-<{}}\n", "", 50); };
}

void Teset1()
{
	std::cout << __func__ << std::endl;

	auto work = [](int time, int id){
		std::this_thread::sleep_for(std::chrono::milliseconds(time + id));
		std::cout << std::format("done: {}\n", id);
		return id * 10;
	};

	//사용 방법이 훨씬 편함.
	//auto f1 = std::async([&]() { return work(200, 0); });
	//int ret1 = f1.get();
	
	// future 를 리턴해서 쉽게 사용 가능.
	auto f1 = std::async([&]() { return work(200, 0); });
	auto f2 = std::async([&]() { return work(200, 1); });
	// 작업 3개가 동시에 진행.
	int ret3 = work(200, 2);
	int ret1 = f1.get();
	int ret2 = f2.get();
	int ret = ret1 + ret2 + ret3;
	std::cout << "restult: " << ret;
}

int main()
{
	PrintSplitLines();
	Teset1();
	return 0;
}