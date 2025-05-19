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

	template< typename RETURN, typename ...ARGUMENTS >
	struct Custom_Task : public Task
	{
		using Packaged_Task = std::packaged_task< RETURN(ARGUMENTS...) >;

		Packaged_Task packaged_task;

		std::tuple<ARGUMENTS...> saved_arguments;

		Custom_Task(RETURN(*function)(const ARGUMENTS & ...), const ARGUMENTS & ...arguments) : packaged_task(function), saved_arguments(arguments...)
		{
		}

		virtual std::any get_future()
		{
			return std::make_shared< std::future<RETURN>>(std::move(packaged_task.get_future()));
		}

		void invoke() override
		{
			std::apply(packaged_task, saved_arguments);
		}
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

	template< typename RETURN, typename ...ARGUMENTS >
	std::shared_ptr < std::future<RETURN>> add_task(RETURN(*function)(const ARGUMENTS & ...), const ARGUMENTS & ...arguments)
	{
		// Is this wasteful?
		std::any unspecified_future;
		{
			std::lock_guard lock(queue_mutex);

			tasks.emplace(std::make_unique< Custom_Task<RETURN, ARGUMENTS...> >(function, arguments...));

			unspecified_future = tasks.back()->get_future();
		}
		assert(unspecified_future.type() == typeid(std::shared_ptr<std::future<RETURN>>));

		// Ask about why I cant just directly return it
		std::shared_ptr<std::future<RETURN>> typed_future = std::any_cast<std::shared_ptr<std::future<RETURN>>>(unspecified_future);

		return typed_future;

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

