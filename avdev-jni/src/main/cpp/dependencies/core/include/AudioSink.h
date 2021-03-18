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

#ifndef AVDEV_CORE_AUDIO_SINK_H_
#define AVDEV_CORE_AUDIO_SINK_H_

#include <cstdint>
#include <memory>

namespace avdev
{
	class AudioSink
	{
		public:
			AudioSink() {}
			virtual ~AudioSink() {}

			virtual void write(const std::uint8_t * data, size_t length, const AudioFormat & format) = 0;

			/* Prevent copy and assignment. */
			AudioSink(const AudioSink & ref) = delete;
			AudioSink & operator=(const AudioSink & ref) = delete;
	};


	using PAudioSink = std::shared_ptr<AudioSink>;
}

#endif