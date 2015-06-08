#pragma once
#include <ppltasks.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <chrono>

template <typename T>
class producer_consumer_queue {
private:
	std::queue<T> _queue;
	std::mutex _mutex;
	std::condition_variable _consumerCondition;
	std::condition_variable _producerCondition;
	std::uint32_t _capacity;
public:
	producer_consumer_queue(std::uint32_t capacity) : _capacity(capacity) {
		
	}
	void enqueue(const T& value, concurrency::cancellation_token cancelToken) {
		std::unique_lock<std::mutex> lg(_mutex);
		while (_queue.size() >= _capacity && !cancelToken.is_canceled()) {
			_producerCondition.wait_for(lg, std::chrono::milliseconds(200));
		}
		if (cancelToken.is_canceled()) {
			throw concurrency::task_canceled();
		}
		_queue.push(value);
		_consumerCondition.notify_one();
	}
	T dequeue(concurrency::cancellation_token cancelToken) {
		std::unique_lock<std::mutex> lg(_mutex);
		while (_queue.size() == 0 && !cancelToken.is_canceled()) {
			_consumerCondition.wait_for(lg, std::chrono::milliseconds(200));
		}

		if (cancelToken.is_canceled()) {
			throw concurrency::task_canceled();
		}

		auto result = _queue.front();
		_queue.pop();
		_producerCondition.notify_one();
		return result;
	}
	void clear() {
		std::unique_lock<std::mutex> lg(_mutex);
		while (_queue.size()>0)
		{
			_queue.pop();
		}
		_producerCondition.notify_one();
	}
};
