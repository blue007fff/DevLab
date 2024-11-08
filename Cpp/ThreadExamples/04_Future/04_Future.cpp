﻿#include <iostream>
#include <thread>
#include <atomic>
#include <format>
#include <string>
#include <future>

//https://modoocode.com/284

void Teset1()
{
	std::cout << __func__ << std::endl;

	// no future
	{
		std::string content;
		std::thread t([](std::string* content) {
			std::cout << "read file... " << std::endl;
			*content = "ABCDEFG";
			}, &content);
		
		t.join();
		std::cout << "content: " << content << std::endl;
	}

	// future
	{
		std::promise<std::string> contentPromise;

		// 미래에 string 데이터를 돌려 주겠다는 약속.
		std::future<std::string> contentFuture = contentPromise.get_future();
		std::thread t([](std::promise<std::string> p) {
			std::cout << "read file... \n";
			p.set_value("ABCDEFG from Future");
			}, std::move(contentPromise));

		// 약속한 데이터를 받을 때 까지 대기.
		// - wait() 리턴은 future 에 데이터가 준비 완료되었다는 의미
		// - get() 이 wait() 을 포함한 것과 같음.
		//contentFuture.wait();

		std::cout << "content: " << contentFuture.get() << std::endl;
		t.join();
	}

}

void Test2_MyAsync()
{
	std::cout << __func__ << std::endl;
	{
		auto MyAsyncGetContent = []()
		{
			std::promise<std::string> contentPromise;
			std::future<std::string> contentFuture = contentPromise.get_future();
			std::thread t([](std::promise<std::string> p) {
				std::cout << "read file... \n";
				p.set_value("ABCDEFG");
				}, std::move(contentPromise));
			t.detach(); // 스레드를 분리하고, 나중에 future 에서 값을 받음.
			return contentFuture;
		};
		std::future<std::string> contentFuture = MyAsyncGetContent();
		std::cout << "content: " << contentFuture.get() << std::endl;
	}

	// 위의 예제를 좀더 유연하게.
	{
		//template <typename T>
		//std::future<T> getContentAsync(std::function<T()> func)
		//람다에서 위의 코드를 다음과 같이 decltype 으로 추론
		//using T = decltype(func()); 

		auto MyAsyncGetContent = [](auto func) {
			using T = decltype(func());  // 함수 반환 타입 추론
			std::promise<T> contentPromise;
			std::future<T> contentFuture = contentPromise.get_future();

			std::thread t([promise = std::move(contentPromise), func]() mutable {
				T content = func();
				promise.set_value(content);
				});
			t.detach();
			return contentFuture;
		};
		std::future<std::string> contentFuture1 = MyAsyncGetContent([]() {
			std::cout << "content1: read file... \n";
			return std::string("ABCDEF"); });
		std::future<std::string> contentFuture2 = MyAsyncGetContent([]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			std::cout << "content2: read file... \n";
			return std::string("FEDCBA"); });

		std::future_status status = contentFuture2.wait_for(std::chrono::milliseconds(10));
		if (status == std::future_status::timeout) {
			std::cout << std::format("content2:status: not ready\n");
		}else if (status == std::future_status::ready) {}

		std::cout << std::format("content1: {}\n", contentFuture1.get());
		std::cout << std::format("content2: {}\n", contentFuture2.get());
	}
}

void Test3_SharedFuture()
{
	std::cout << __func__ << std::endl;
	auto runner = [](std::shared_future<void> start, int id) {
		start.get();
		std::cout << std::format("start: {}\n", id);
	};

	std::promise<void> p;
	std::shared_future<void> start = p.get_future();

	std::vector<std::thread> threads;
	for (int i = 0; i < 5; ++i) {
		threads.emplace_back(runner, start, i);
	}
	//cerr 는 std::cout 과는 다르게 버퍼를 사용하지 않기 때문에 터미널에 바로 출력된다.
	std::cerr << "ready...";
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cerr << "go!" << std::endl;

	p.set_value();

	for (auto& thd : threads) thd.join();
}

//https://en.cppreference.com/w/cpp/thread/packaged_task
void Test4_PackagedTask()
{
	std::cout << __func__ << std::endl;

	// task
	{			
		// promise 와 다르게 함수 자체를 받음.
		std::packaged_task<int(int, int)> task([](int a, int b) {return a + b; });
		std::future<int> result = task.get_future();
		task(11, 22);
		std::cout << "task: " << result.get() << std::endl;

		task.reset();  // 재사용 초기화
		std::future<int> result2 = task.get_future();
		task(33, 44);  // 두 번째 호출
		std::cout << "task-reuse: " << result2.get() << std::endl;
	}

	// task-bind
	{
		std::packaged_task<int()> task(std::bind([](int a, int b) {return a + b; }, 11, 22));
		std::future<int> result = task.get_future();
		task();
		std::cout << "task-bind: " << result.get() << std::endl;
	}

	// task-thread
	{
		std::packaged_task<int(int, int)> task([](int a, int b) {return a + b; });
		std::future<int> workFuture = task.get_future();

		// function 대신 task 를 넘겨 줌, task() 호출을 thread 로 전달.
		std::thread t(std::move(task), 11, 22);

		std::cout << "task-thread:: " << workFuture.get() << std::endl;
		t.join();
	}	
}

#include <type_traits> // C++20 이상에서 std::result_of_t -> std::invoke_result_t 사용

template<typename Func, typename... Args>
std::future<std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>> 
MyPackagedTaskAsync(Func&& func, Args... args) {
	using ReturnType = std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>;
	std::cout << std::format("ReturnType:{}, Func:{}, sizeof(Func):{}\n", 
		typeid(ReturnType).name(), typeid(Func).name(), sizeof(func));

	//std::packaged_task<Result()> task(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
	std::packaged_task<ReturnType()> task(std::bind(func, std::forward<Args>(args)...));

	auto taskFuture = task.get_future();
	std::thread t([&task = task]() {task(); });
	t.join();
	return taskFuture;
}

void Test4_PackagedTask_MyAsync()
{
	std::cout << __func__ << std::endl;

	auto future1 = MyPackagedTaskAsync([](int a, int b) { return a + b; }, 10, 20);
	std::cout << "MyAsync: " << future1.get() << std::endl;

	int a = 0, b = 0;
	auto future2 = MyPackagedTaskAsync([a, b](int c) { return c * c; }, 10);
	std::cout << "MyAsync: " << future2.get() << std::endl;
}

int main()
{
	auto PrintSplitLines = []() {std::cout << std::format("{:-<{}}\n", "", 50); };

	PrintSplitLines();
	Teset1();

	PrintSplitLines();
	Test2_MyAsync();

	PrintSplitLines();
	Test3_SharedFuture();

	PrintSplitLines();
	Test4_PackagedTask();

	PrintSplitLines();
	Test4_PackagedTask_MyAsync();
	return 0;
}