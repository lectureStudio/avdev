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

#ifndef AVDEV_CORE_AUDIO_STREAM_H_
#define AVDEV_CORE_AUDIO_STREAM_H_

#include "Stream.h"
#include "AudioFormat.h"
#include "AudioSessionListener.h"
#include "RingBuffer.h"

#include <cstdint>
#include <list>
#include <memory>

namespace avdev
{
	class AudioStream : public Stream
	{
		public:
			virtual ~AudioStream() {};

			void attachSessionListener(PAudioSessionListener listener);
			void detachSessionListener(PAudioSessionListener listener);

			virtual float getVolume();
			virtual void setVolume(float volume);

			virtual bool getMute();
			virtual void setMute(bool mute);

			virtual void setAudioFormat(AudioFormat format);
			virtual AudioFormat const& getAudioFormat() const;

			virtual void setBufferLatency(unsigned latency);
			virtual unsigned getBufferLatency();

			size_t getStreamPosition();

		protected:
			AudioStream();

			void initAudioBuffer(size_t length);
			void freeAudioBuffer();

			void setStreamPosition(size_t pos);

			void notifyVolumeChange(float volume, bool mute);

			ByteBuffer ioBuffer;
			RingBuffer<std::uint8_t> streamBuffer;

		private:
			AudioFormat audioFormat;
			unsigned bufferLatency;
			float volume;
			bool mute;
			size_t streamPos;
			std::list<std::weak_ptr<AudioSessionListener>> sessionListeners;
	};
}

#endif