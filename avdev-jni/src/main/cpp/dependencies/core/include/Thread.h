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

#ifndef AVDEV_CORE_THREAD_H_
#define AVDEV_CORE_THREAD_H_

#include <atomic>
#include <thread>

namespace avdev
{
	class Thread
	{
		public:
			Thread();
			virtual ~Thread();

			Thread(Thread const&) = delete;
			Thread & operator =(Thread const&) = delete;

		protected:
			void startThread();
			void stopThread();
			void stopThreadAndWait();
			bool isRunning();

			virtual void run() = 0;

		private:
			std::thread thread;
			std::atomic<bool> running;
	};
}

#endif