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

#include "AudioInputStream.h"

namespace avdev
{
	AudioInputStream::AudioInputStream(PAudioSource source) :
		AudioStream(),
		source(source),
		playbackBufferMs(2000)
	{
	}

	void AudioInputStream::setPlaybackBufferPeriod(unsigned millis)
	{
		this->playbackBufferMs = millis;
	}

	unsigned AudioInputStream::getPlaybackBufferSize()
	{
		AudioFormat format = getAudioFormat();
		unsigned size = (format.getSampleRate() * format.getChannels() * (format.bitsPerSample() / 8) *
			playbackBufferMs) / 1000;

		return size;
	}

	void AudioInputStream::initAudioBuffer(size_t length)
	{
		AudioStream::initAudioBuffer(length);

		length *= 5;

		if (buffer.size() != length) {
			buffer.resize(length);
		}
	}

	int AudioInputStream::readAudio(size_t length)
	{
		if (source == nullptr) {
			return -1;
		}

		// Read samples from the playback buffer.
		size_t read = streamBuffer.read(ioBuffer.data(), length);

		if (read < length) {
			// Insufficient number of samples read from the buffer. Read the rest from the source.
			int readSource = source->read(ioBuffer.data() + read, 0, length - read);

			if (readSource < 0) {
				// End of stream.
				ended();
				return -1;
			}

			if (readSource > 0) {
				// Fill up the playback buffer with new samples.
				int sourceBufferLen = source->read(buffer.data(), 0, buffer.size());

				if (sourceBufferLen < 0) {
					// End of stream.
					ended();
					return -1;
				}

				if (sourceBufferLen > 0) {
					streamBuffer.write(buffer.data(), sourceBufferLen);
				}

				read += readSource;

				if (read < length) {
					// Insufficient number of samples read. Fill the rest with zeros (silence).
					std::fill_n(ioBuffer.begin() + read, length - read, 0);
				}
			}
		}

		// Update stream position.
		setStreamPosition(getStreamPosition() + read);

		//printf("ring: %d - %d - %d \n", length, read, streamBuffer.getAvailable());
		//fflush(NULL);

		return static_cast<int>(read);
	}
}
