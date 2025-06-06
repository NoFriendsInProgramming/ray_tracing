﻿// Author: Alex Ruiz & Angel Rodriguez 2025
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

namespace udit::concurrencytools
{
	class ThreadPool
	{
	private:

		struct Task
		{
			virtual ~Task() = default;

			virtual void invoke() = 0;

			virtual std::any get_future() = 0;

		};

		template <typename T>
		struct forward_capture {
			T value;
			explicit forward_capture(T&& val) : value(std::forward<T>(val)) {}

			T&& forward() { return std::move(value); }
		};

		template <typename T>
		struct forward_capture<T&> {
		private:
			static constexpr bool should_copy =
				std::is_copy_constructible_v<T> &&
				(std::is_fundamental_v<T> || std::is_trivially_copyable_v<T>);

		public:
			using StorageType = std::conditional_t<should_copy, T, T*>;

			StorageType storage;

			explicit forward_capture(T& val)
				: storage(init_storage(val)) {
			}

			T& forward() {
				if constexpr (should_copy) {
					return storage;
				}
				else {
					return *storage;
				}
			}

		private:
			static StorageType init_storage(T& val) {
				if constexpr (should_copy) {
					return val; 
				}
				else {
					return &val;
				}
			}
		};


		template <typename Callable, typename... Args>
		class Custom_Task : public Task {
		public:
			using ReturnType = std::invoke_result_t<Callable, Args...>;
			using PackagedTask = std::packaged_task<ReturnType()>;

			explicit Custom_Task(Callable&& new_func, Args&&... args)
				: func(std::forward<Callable>(new_func)),
				captures(forward_capture<Args>(std::forward<Args>(args))...)
			{
				auto bound = [this]() mutable -> ReturnType {
					return invoke_impl(std::index_sequence_for<Args...>{});
					};

				packaged_task = PackagedTask(std::move(bound));
			}

			std::any get_future() override {
				return std::make_shared<std::future<ReturnType>>(packaged_task.get_future());
			}

			void invoke() override {
				packaged_task();
			}

		private:
			Callable func;
			std::tuple<forward_capture<Args>...> captures;
			PackagedTask packaged_task;

			template <std::size_t... I>
			ReturnType invoke_impl(std::index_sequence<I...>) {
				return std::invoke(func, std::get<I>(captures).forward()...);
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
			stop_threads();
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
			task_cv.notify_all();

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

	inline void ThreadPool::thread_function(std::stop_token stop_token)
	{
		std::unique_lock lock(queue_mutex);
		while (!stop_token.stop_requested())
		{

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
}

