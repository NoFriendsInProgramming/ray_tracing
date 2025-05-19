// Author: Alex Ruiz & Angel Rodriguez 2025
// Public Domain

#include <ThreadPool.hpp>

void ThreadPool::thread_function(std::stop_token stop_token, int index)
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
			//std::cout << -index << ", ";
			lock.lock();
		}
	}
}