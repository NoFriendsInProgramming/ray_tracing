// Author: Alex Ruiz & Angel Rodriguez 2025
// Public Domain

#pragma once


#include <cassert>
#include <memory>
#include <thread>
#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <tuple>
#include <any>
#include <iostream>

class ThreadPool
{
private:

	struct Task
	{
		virtual ~Task() = default;

		virtual void invoke() = 0;

		virtual std::any get_future() = 0;

	protected:
		Task() = default;
	};

	template <typename Callable, typename... Args>
	class Custom_Task : public Task {
	public:
		using ReturnType = std::invoke_result_t<Callable, Args...>;
		using PackagedTask = std::packaged_task<ReturnType()>;

		explicit Custom_Task(Callable&& func, Args&&... args)
			: bound_task(std::bind(std::forward<Callable>(func), std::forward<Args>(args)...)),
			packaged_task(bound_task) {
		}

		std::any get_future() override {
			return std::make_shared<std::future<ReturnType>>(packaged_task.get_future());
		}

		void invoke() override {
			packaged_task();  // no arguments here, it's already bound
		}

	private:
		std::function<ReturnType()> bound_task;
		PackagedTask packaged_task;
	};




private:

	int cores;

	std::mutex queue_mutex;

	std::condition_variable task_cv;

	std::queue< std::unique_ptr< Task > > tasks;

	std::vector< std::unique_ptr< std::jthread > > threads;

	std::atomic< bool > is_running;

public:

	ThreadPool()
	{
		cores = std::thread::hardware_concurrency();
		is_running = false;
	}

	~ThreadPool()
	{
		// I do not think this is really necessary though... I guess I have trouble understanding memory management still
		stop_threads();
		// cleanup: stop(), ...
	}

	template <typename Callable, typename... Args>
	auto add_task(Callable&& func, Args&&... args)
		-> std::shared_ptr<std::future<std::invoke_result_t<Callable, Args...>>>
	{
		using ReturnType = std::invoke_result_t<Callable, Args...>;

		std::any unspecified_future;
		{
			std::lock_guard lock(queue_mutex);

			tasks.emplace(std::make_unique<
				Custom_Task<Callable, Args...>>(
					std::forward<Callable>(func),
					std::forward<Args>(args)...));

			unspecified_future = tasks.back()->get_future();
		}

		return std::any_cast<std::shared_ptr<std::future<ReturnType>>>(unspecified_future);
	}


	void start()
	{
		assert(!is_running);

		threads.resize(cores);

		for (auto& thread : threads)
		{
			thread = std::make_unique< std::jthread >
				(
					[this](std::stop_token token) { this->thread_function(token); }
				);
		}

		is_running = true;
	}

	void stop()
	{
		assert(is_running);

		stop_threads();

		threads.clear();

		is_running = false;
	}

private:

	void stop_threads()
	{
		for (auto& thread : threads)
		{
			thread->request_stop();
			thread->join();
		}
	}
	void thread_function(std::stop_token);

};

void ThreadPool::thread_function(std::stop_token stop_token)
{
	std::unique_lock lock(queue_mutex);
	while (!stop_token.stop_requested())
	{

		// Wait until there are tasks to process or stop has been requested
		task_cv.wait(lock, [&] { return (stop_token.stop_requested()) || !tasks.empty(); });

		if (stop_token.stop_requested()) {
			break;
		}

		while (!tasks.empty())
		{
			auto task = std::move(tasks.front());
			tasks.pop();
			lock.unlock();
			task->invoke();
			lock.lock();
		}
	}
}

