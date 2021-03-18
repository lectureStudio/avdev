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

#ifndef AVDEV_CORE_AUDIO_INPUT_STREAM_H_
#define AVDEV_CORE_AUDIO_INPUT_STREAM_H_

#include "AudioStream.h"
#include "AudioSource.h"
#include <memory>

namespace avdev
{
	class AudioInputStream : public AudioStream
	{
		public:
			AudioInputStream(PAudioSource source);
			virtual ~AudioInputStream() {};

			void setPlaybackBufferPeriod(unsigned millis);

		protected:
			int readAudio(size_t length);
			unsigned getPlaybackBufferSize();

			void initAudioBuffer(size_t length);

			PAudioSource source;

		private:
			unsigned playbackBufferMs;
	};


	using PAudioInputStream = std::unique_ptr<AudioInputStream>;
}

#endif