/*
 * Copyright 2016 Alex Andres
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AVDEV_CORE_RING_BUFFER_H_
#define AVDEV_CORE_RING_BUFFER_H_

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstring>

namespace avdev
{
	template <typename Type>
	class RingBuffer
	{
		public:
			RingBuffer(const size_t capacity) : capacity(capacity), head(0), tail(0) {
				buffer = new Type[capacity];
			}

			virtual ~RingBuffer() {
				delete[] buffer;
			}

			size_t write(const Type * data, size_t length) {
				size_t head_a = head.load(std::memory_order_relaxed);
				size_t tail_a = tail.load(std::memory_order_relaxed);

				length = std::min(length, getFree(head_a, tail_a));
				const size_t write = std::min(length, capacity - head_a);

				std::memcpy(buffer + head_a, data, write);
				std::memcpy(buffer, data + write, length - write);

				updateIndex(head, head_a, length);

				return length;
			}

			size_t read(Type * data, size_t length) {
				size_t head_a = head.load(std::memory_order_relaxed);
				size_t tail_a = tail.load(std::memory_order_relaxed);

				length = std::min(length, getUsed(head_a, tail_a));
				const size_t read = std::min(length, capacity - tail_a);

				std::memcpy(data, buffer + tail_a, read);
				std::memcpy(data + read, buffer, length - read);

				updateIndex(tail, tail_a, length);

				return length;
			}

			size_t getAvailable()
			{
				size_t head_a = head.load(std::memory_order_relaxed);
				size_t tail_a = tail.load(std::memory_order_relaxed);

				return getUsed(head_a, tail_a);
			}

			void reset()
			{
				head.store(0, std::memory_order_release);
				tail.store(0, std::memory_order_release);
			}

		private:
			size_t getFree(size_t & head, size_t & tail)
			{
				if (tail > head) {
					return tail - head;
				}
				else {
					return capacity - head + tail;
				}
			}

			size_t getUsed(size_t & head, size_t & tail)
			{
				if (head >= tail) {
					return head - tail;
				}
				else {
					return capacity - tail + head;
				}
			}

			void updateIndex(std::atomic<size_t> & index, size_t indexValue, size_t length)
			{
				if (length >= capacity - indexValue) {
					index.store(indexValue + length - capacity, std::memory_order_release);
				}
				else {
					index.store(indexValue + length, std::memory_order_release);
				}
			}

			const size_t capacity;
			Type * buffer;
			std::atomic<size_t> head;
			std::atomic<size_t> tail;
	};
}

#endif