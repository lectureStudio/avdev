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

#include "AVdevException.h"
#include "Stream.h"
#include "Log.h"
#include "MessageQueue.h"

namespace avdev
{
	Stream::Stream() :
		buffer(),
		state(StreamState::CLOSED)
	{
	}

	void Stream::open()
	{
		std::lock_guard<std::recursive_mutex> guard(mutex);

		checkState(StreamState::OPENED);
		openInternal();
		setState(StreamState::OPENED);
	}

	void Stream::close()
	{
		std::lock_guard<std::recursive_mutex> guard(mutex);

		checkState(StreamState::CLOSED);
		closeInternal();
		setState(StreamState::CLOSED);
	}

	void Stream::start()
	{
		std::lock_guard<std::recursive_mutex> guard(mutex);
		
		checkState(StreamState::STARTED);
		startInternal();
		setState(StreamState::STARTED);
	}

	void Stream::stop()
	{
		std::lock_guard<std::recursive_mutex> guard(mutex);

		checkState(StreamState::STOPPED);
		stopInternal();
		setState(StreamState::STOPPED);
	}

	void Stream::ended()
	{
		std::lock_guard<std::recursive_mutex> guard(mutex);

		checkState(StreamState::ENDED);
		setState(StreamState::ENDED);
	}

	void Stream::attachStreamListener(PStreamListener listener)
	{
		streamListeners.push_back(listener);
	}

	void Stream::detachStreamListener(PStreamListener listener)
	{
		streamListeners.remove_if([listener](std::weak_ptr<StreamListener> p) {
			return !(p.owner_before(listener) || listener.owner_before(p));
		});
	}

	void Stream::checkState(StreamState nextState)
	{
		std::string stateStr = getStateString(nextState);

		switch (getState()) {
			case StreamState::CLOSED:
				if (nextState != StreamState::OPENED)
					throw AVdevException("Invalid stream state transition: CLOSED -> %s", stateStr.c_str());
				break;

			case StreamState::STOPPED:
				if (nextState != StreamState::STARTED && nextState != StreamState::CLOSED)
					throw AVdevException("Invalid stream state transition: STOPPED -> %s", stateStr.c_str());
				break;

			case StreamState::STARTED:
				if (nextState != StreamState::STOPPED && nextState != StreamState::ENDED && nextState != StreamState::CLOSED)
					throw AVdevException("Invalid stream state transition: STARTED -> %s", stateStr.c_str());
				break;

			case StreamState::OPENED:
				if (nextState != StreamState::STARTED && nextState != StreamState::CLOSED)
					throw AVdevException("Invalid stream state transition: OPENED -> %s", stateStr.c_str());
				break;

			case StreamState::ENDED:
				if (nextState != StreamState::STOPPED && nextState != StreamState::CLOSED)
					throw AVdevException("Invalid stream state transition: ENDED -> %s", stateStr.c_str());
				break;

			default:
				throw AVdevException("Unknown stream state transition.");
		}
	}

	std::string Stream::getStateString(StreamState state)
	{
		switch (state) {
			case StreamState::CLOSED:
				return "CLOSED";
			case StreamState::STOPPED:
				return "STOPPED";
			case StreamState::STARTED:
				return "STARTED";
			case StreamState::OPENED:
				return "OPENED";
			case StreamState::ENDED:
				return "ENDED";
			default:
				throw AVdevException("Unknown device state.");
		}
	}

	void Stream::setState(StreamState state)
	{
		std::lock_guard<std::recursive_mutex> guard(mutex);

		this->state = state;

		std::string stateStr = getStateString(state);

		LOGDEV_INFO("Stream state: %s.", stateStr.c_str());

		notifyListeners(state);
	}

	StreamState Stream::getState() const
	{
		return state;
	}

	void Stream::initBuffer(size_t length)
	{
		if (buffer.size() != length) {
			buffer.resize(length);
		}
	}

	void Stream::freeBuffer()
	{
		buffer.clear();
		buffer.shrink_to_fit();
	}

	void Stream::writeBuffer(const std::uint8_t * data, size_t length)
	{
		if (buffer.empty()) {
			initBuffer(length);
		}

		std::memcpy(buffer.data(), (const void *)data, length);
	}

	void Stream::notifyListeners(StreamState & state)
	{
		for (auto i = streamListeners.begin(); i != streamListeners.end();) {
			if ((*i).expired()) {
				i = streamListeners.erase(i);
			}
			else {
				PStreamListener listener = (*i).lock();

				MessageQueue & mq = MessageQueue::instance();

				switch (state) {
					case StreamState::OPENED:
						mq.dispatch([listener]() { listener->streamOpened(); });
						break;
					case StreamState::CLOSED:
						mq.dispatch([listener]() { listener->streamClosed(); });
						break;
					case StreamState::STARTED:
						mq.dispatch([listener]() { listener->streamStarted(); });
						break;
					case StreamState::STOPPED:
						mq.dispatch([listener]() { listener->streamStopped(); });
						break;
					case StreamState::ENDED:
						mq.dispatch([listener]() { listener->streamEnded(); });
						break;
				}

				++i;
			}
		}
	}
}