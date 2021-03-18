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

#include "MessageQueue.h"
#include "AVdevException.h"
#include "Log.h"

namespace avdev
{
	MessageQueue & MessageQueue::instance()
	{
		static MessageQueue instance;

		return instance;
	}

	void MessageQueue::dispatch(std::function<void()> func)
	{
		queue.push(func);
	}

	void MessageQueue::start()
	{
		if (running) {
			return;
		}

		running = true;

		thread = std::thread(&MessageQueue::run, this);
	}

	void MessageQueue::stop()
	{
		if (!running) {
			return;
		}

		// Shutdown gracefully.
		running = false;

		queue.clear();

		// Poison pill.
		dispatch([]() {});

		if (thread.joinable()) {
			thread.join();
		}
	}


	MessageQueue::MessageQueue() :
		running(false)
	{
	}

	MessageQueue::~MessageQueue()
	{
		stop();
	}

	void MessageQueue::run()
	{
		while (running) {
			std::function<void()> func = queue.pop();

			try {
				func();
			}
			catch (AVdevException& ex) {
				LOGDEV_ERROR("MessageQueue: Exception caught: %s.", ex.what());
			}
			catch (...) {
				LOGDEV_ERROR("MessageQueue: Unknown exception caught.");
			}
		}
	}
}