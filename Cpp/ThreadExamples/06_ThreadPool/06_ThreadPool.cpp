#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <utility>
#include <format>
//https://modoocode.com/285


namespace ThreadPool
{
	class ThreadPool
	{
	public:
		ThreadPool(size_t numThread);
		~ThreadPool();

		// job 을 추가한다.
		template <class F, class... Args>
		std::future<std::invoke_result_t<F, Args...>>
			EnqueueJob(F&& f, Args&&... args);

	private:		
		void WorkerThread(); // Worker 쓰레드

	private:
		std::vector<std::thread> m_workerThreads;

		// 작업 보관
		std::queue<std::function<void()>> m_jobs;
		std::condition_variable m_cvForJobs;
		std::mutex m_mutexForJobs;

		// 모든 쓰레드 종료
		bool m_stopAll{ false };

	};

	ThreadPool::ThreadPool(size_t numThread)
	{
		m_workerThreads.reserve(numThread);
		for (size_t i = 0; i < numThread; ++i) {
			m_workerThreads.emplace_back([this]() { this->WorkerThread(); });
		}
	}

	ThreadPool::~ThreadPool()
	{
		m_stopAll = true;
		m_cvForJobs.notify_all();
		for (auto& t : m_workerThreads) {
			t.join();
		}
	}

	void ThreadPool::WorkerThread() 
	{
		while (true) 
		{
			std::unique_lock<std::mutex> lock(m_mutexForJobs);
			m_cvForJobs.wait(lock, [this]() { return !this->m_jobs.empty() || m_stopAll; });
			if (m_stopAll && this->m_jobs.empty()) {
				// 전체 중단 및 작업이 없는 경우 종료.
				return;
			}			
			std::function<void()> job = std::move(m_jobs.front());
			m_jobs.pop();
			lock.unlock();
						
			job();
		}
	}

	template <class F, class... Args>
	std::future<std::invoke_result_t<F, Args...>>
		ThreadPool::EnqueueJob(F&& f, Args&&... args)
	{
		if (m_stopAll) {
			throw std::runtime_error("ThreadPool 사용 중지됨");
		}

		using ReturnType = std::invoke_result_t<F, Args...>;
		auto job = std::make_shared<std::packaged_task<ReturnType()>>(
			std::bind(f, std::forward<Args>(args)...));

		std::future<ReturnType> job_result_future = job->get_future();
		{
			std::lock_guard<std::mutex> lock(m_mutexForJobs);
			m_jobs.push([job]() { (*job)(); });
		}
		m_cvForJobs.notify_one();

		return job_result_future;
	}

}  // namespace ThreadPool

int work(int t, int id) 
{
	printf("%d start \n", id);
	std::this_thread::sleep_for(std::chrono::seconds(t));
	printf("%d end after %ds\n", id, t);
	return id;
}

int main() 
{
	ThreadPool::ThreadPool pool(3);

	std::vector<std::future<int>> futures;
	for (int i = 0; i < 10; i++) {
		futures.emplace_back(pool.EnqueueJob(work, i % 3 + 1, i));
	}
	for (auto& f : futures) {
		printf("result : %d \n", f.get());
	}
}
