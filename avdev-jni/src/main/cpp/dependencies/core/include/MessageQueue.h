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

#ifndef AVDEV_CORE_MESSAGE_QUEUE_H_
#define AVDEV_CORE_MESSAGE_QUEUE_H_

#include "Queue.h"

#include <atomic>
#include <functional>
#include <thread>

namespace avdev
{
	class MessageQueue
	{
		public:
			static MessageQueue & instance();

			void dispatch(std::function<void()> func);

			void start();
			void stop();

			// Delete copy and move constructors and assign operators.
			MessageQueue(const MessageQueue &) = delete;
			MessageQueue(MessageQueue &&) = delete;
			MessageQueue & operator=(const MessageQueue &) = delete;
			MessageQueue & operator=(MessageQueue &&) = delete;

		private:
			MessageQueue();
			~MessageQueue();

			void run();

		private:
			std::atomic<bool> running;
			std::thread thread;
			Queue<std::function<void()>> queue;
	};
}

#endif