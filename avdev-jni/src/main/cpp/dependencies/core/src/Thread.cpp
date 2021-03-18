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

#include "Thread.h"
#include <system_error>

namespace avdev
{
	Thread::Thread() :
		thread(),
		running(false)
	{
	}

	Thread::~Thread()
	{
		try {
			stopThread();
		}
		catch (std::system_error const& error) {
			printf("Thread Error: Failed to stop thread: %s.\n", error.what());
		}
		catch (...) {
			printf("Thread Error: An error occurred while stopping thread.\n");
		}
	}

	void Thread::startThread()
	{
		running = true;

		thread = std::thread(&Thread::run, this);
	}

	void Thread::stopThread()
	{
		running = false;
	}

	void Thread::stopThreadAndWait()
	{
		stopThread();

		if (thread.joinable()) {
			try {
				thread.join();
			}
			catch (const std::system_error & error) {
				printf("Thread Join Error: %s.\n", error.what());
			}
		}
	}

	bool Thread::isRunning()
	{
		return running;
	}
}