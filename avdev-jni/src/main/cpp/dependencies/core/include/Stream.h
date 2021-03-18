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

#ifndef AVDEV_CORE_STREAM_H_
#define AVDEV_CORE_STREAM_H_

#include "avdev.h"
#include "Exception.h"
#include "StreamListener.h"

#include <list>
#include <mutex>
#include <memory>

namespace avdev
{
	enum class StreamState {
		CLOSED, OPENED, STARTED, STOPPED, ENDED
	};

	class Stream
	{
		public:
			virtual ~Stream() {};

			void open();
			void close();
			void start();
			void stop();

			void attachStreamListener(PStreamListener listener);
			void detachStreamListener(PStreamListener listener);

		protected:
			Stream();

			void ended();

			virtual void openInternal() = 0;
			virtual void closeInternal() = 0;
			virtual void startInternal() = 0;
			virtual void stopInternal() = 0;

			static std::string getStateString(StreamState state);
			void checkState(StreamState nextState);
			void setState(StreamState state);
			StreamState getState() const;

			void initBuffer(size_t length);
			void freeBuffer();
			void writeBuffer(const std::uint8_t * data, size_t length);

			ByteBuffer buffer;

		private:
			void notifyListeners(StreamState & state);

			std::recursive_mutex mutex;

			StreamState state;

			std::list<std::weak_ptr<StreamListener>> streamListeners;
	};
}

#endif