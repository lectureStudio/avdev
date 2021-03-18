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

#include "AudioStream.h"
#include "AVdevException.h"

namespace avdev
{
	AudioStream::AudioStream() : Stream(),
        ioBuffer(),
		streamBuffer(1024 * 1024),
		audioFormat(AudioFormat(SampleFormat::S16LE, 44100, 1)),
		bufferLatency(20),
		volume(1.0),
		mute(false),
		streamPos(0)
	{
	}

	void AudioStream::attachSessionListener(PAudioSessionListener listener)
	{
		sessionListeners.push_back(listener);
	}

	void AudioStream::detachSessionListener(PAudioSessionListener listener)
	{
		sessionListeners.remove_if([listener](std::weak_ptr<AudioSessionListener> p) {
			return !(p.owner_before(listener) || listener.owner_before(p));
		});
	}

	void AudioStream::setVolume(float volume)
	{
		if (volume < 0 || volume > 1) {
			throw AVdevException("Volume has to be in range [0,1]. Given %f.", volume);
		}

		this->volume = volume;
	}

	float AudioStream::getVolume()
	{
		return volume;
	}

	void AudioStream::setMute(bool mute)
	{
		this->mute = mute;
	}

	bool AudioStream::getMute()
	{
		return mute;
	}

	void AudioStream::setAudioFormat(AudioFormat format)
	{
		this->audioFormat = format;
	}

	AudioFormat const& AudioStream::getAudioFormat() const
	{
		return audioFormat;
	}

	void AudioStream::setBufferLatency(unsigned latency)
	{
		this->bufferLatency = latency;
	}

	unsigned AudioStream::getBufferLatency()
	{
		return bufferLatency;
	}

	size_t AudioStream::getStreamPosition()
	{
		return streamPos;
	}

	void AudioStream::initAudioBuffer(size_t length)
	{
		if (ioBuffer.size() != length) {
			ioBuffer.resize(length);
		}
	}

	void AudioStream::freeAudioBuffer()
	{
		ioBuffer.clear();
		ioBuffer.shrink_to_fit();

		streamBuffer.reset();
	}

	void AudioStream::setStreamPosition(size_t pos)
	{
		this->streamPos = pos;
	}

	void AudioStream::notifyVolumeChange(float volume, bool mute)
	{
		for (auto i = sessionListeners.begin(); i != sessionListeners.end();) {
			if ((*i).expired()) {
				i = sessionListeners.erase(i);
			}
			else {
				PAudioSessionListener listener = (*i).lock();

				listener->volumeChanged(volume, mute);

				//MessageQueue & mq = MessageQueue::instance();
				//mq.dispatch([listener, volume, mute]() { listener->volumeChanged(volume, mute); });

				++i;
			}
		}
	}
}
