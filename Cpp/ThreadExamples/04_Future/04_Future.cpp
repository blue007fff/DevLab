#include <iostream>
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

void Test4_PackagedTask()
{
	std::cout << __func__ << std::endl;

	// promise 와 다르게 함수 자체를 받음.
	std::packaged_task<int(int, int)> task([](int a, int b) {return a + b; });
	std::future<int> workFuture = task.get_future();

	// function 대신 task 를 넘겨 줌.
	std::thread t(std::move(task), 10, 20);

	std::cout << "result : " << workFuture.get() << std::endl;
	t.join();
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
	return 0;
}