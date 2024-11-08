#include <chrono>  // std::chrono::miliseconds
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <format>
#include <thread>
#include <vector>

// https://modoocode.com/270 참고

namespace test1
{
	void producer(std::queue<std::string>* downloaded_pages, std::mutex* m, int index)
	{
		for (int i = 0; i < 5; i++)
		{
			// 웹사이트를 다운로드 하는데 걸리는 시간이라 생각하면 된다.
			// 각 쓰레드 별로 다운로드 하는데 걸리는 시간이 다르다.
			std::this_thread::sleep_for(std::chrono::milliseconds(100 * index));
			std::string content = std::format("data : {} : from thread({})", i, index);
			std::cout << std::format("{}\n", content);

			// data 는 쓰레드 사이에서 공유되므로 critical section 에 넣어야 한다.
			m->lock();
			downloaded_pages->push(content);
			m->unlock();
		}
	}

	void consumer(std::queue<std::string>* downloaded_pages, std::mutex* m, int* num_processed)
	{
		// 전체 처리하는 페이지 개수가 5 * 5 = 25 개.
		while (*num_processed < 25)
		{
			m->lock();
			// 만일 현재 다운로드한 페이지가 없다면 다시 대기.
			if (downloaded_pages->empty()) {
				m->unlock();  // (Quiz) 여기서 unlock 을 안한다면 어떻게 될까요?

				// 놀고 있을 확률이 높아서 쉬어줘야함
				// 10 밀리초 뒤에 다시 확인한다.
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			// 맨 앞의 페이지를 읽고 대기 목록에서 제거한다.
			std::string content = downloaded_pages->front();
			downloaded_pages->pop();

			(*num_processed)++;
			m->unlock();

			// content 를 처리한다.
			std::cout << std::format("{} processed.\n", content);
			std::this_thread::sleep_for(std::chrono::milliseconds(80));
		}
	}

	void test()
	{
		std::cout << __func__ << std::endl;

		// producer 에서 작업을 가져오고, consumer 에서 작업을 처리.
		// producer 보다 consumer 의 작업 처리가 빠른 상황.
		std::queue<std::string> downloaded_pages;
		std::mutex m;

		std::vector<std::thread> producers;
		for (int i = 0; i < 5; i++) {
			producers.push_back(std::thread(producer, &downloaded_pages, &m, i + 1));
		}

		int num_processed = 0;
		std::vector<std::thread> consumers;
		for (int i = 0; i < 3; i++) {
			consumers.push_back(std::thread(consumer, &downloaded_pages, &m, &num_processed));
		}

		for (int i = 0; i < 5; i++) { producers[i].join(); }
		for (int i = 0; i < 3; i++) { consumers[i].join(); }
	}
}

#include <condition_variable>  // std::condition_variable
namespace test2_cv
{
	void producer(std::queue<std::string>& downloaded_pages, std::mutex* m, int index, std::condition_variable* cv)
	{
		for (int i = 0; i < 5; i++)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100 * index));
			std::string content = std::format("data : {} : from thread({})", i, index);
			std::cout << std::format("{}\n", content);

			// data 는 쓰레드 사이에서 공유되므로 critical section 에 넣어야 한다.			
			std::unique_lock<std::mutex> lock(*m);
			downloaded_pages.push(content);
			cv->notify_one();
		}
	}

	void consumer(std::queue<std::string>& downloaded_pages, std::mutex* m, int* num_processed, std::condition_variable* cv)
	{
		// 전체 처리하는 페이지 개수가 5 * 5 = 25 개.
		while (*num_processed < 25)
		{
			std::unique_lock<std::mutex> lock(*m);

			// wait 동안 lock 을 해제
			cv->wait(lock, [&] { return !downloaded_pages.empty() || *num_processed == 25; });

			if (*num_processed == 25) {
				lock.unlock();
				return;
			}

			// 맨 앞의 페이지를 읽고 대기 목록에서 제거한다.
			std::string content = downloaded_pages.front();
			downloaded_pages.pop();

			(*num_processed)++;
			lock.unlock();

			// content 를 처리한다.
			std::cout << std::format("{} processed.\n", content);
			std::this_thread::sleep_for(std::chrono::milliseconds(80));
		}
	}

	void test()
	{
		std::cout << __func__ << std::endl;

		std::queue<std::string> downloaded_pages;
		std::mutex m;
		std::condition_variable cv;

		std::vector<std::thread> producers;
		for (int i = 0; i < 5; i++) {
			producers.push_back(std::thread(producer, std::ref(downloaded_pages), &m, i + 1, &cv));
		}

		int num_processed = 0;
		std::vector<std::thread> consumers;
		for (int i = 0; i < 3; i++) {
			consumers.push_back(
				std::thread(consumer, std::ref(downloaded_pages), &m, &num_processed, &cv));
		}

		for (int i = 0; i < 5; i++) { producers[i].join(); }
		cv.notify_all();
		for (int i = 0; i < 3; i++) { consumers[i].join(); }
	}
}

int main()
{
	auto PrintSplitLines = []() {std::cout << std::format("{:-<{}}\n", "", 50); };

	PrintSplitLines();
	test1::test();

	PrintSplitLines();
	test2_cv::test();
}
