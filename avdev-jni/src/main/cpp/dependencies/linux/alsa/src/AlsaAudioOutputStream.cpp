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

#include "AlsaAudioOutputStream.h"
#include "Log.h"
#include <cmath>

namespace avdev
{
	AlsaAudioOutputStream::AlsaAudioOutputStream(std::string deviceId, PAudioSink sink) :
		AlsaAudioStream(deviceId),
		AudioOutputStream(sink)
	{
	}

	float AlsaAudioOutputStream::getVolume()
	{
		return AlsaAudioStream::getVolume();
	}

	void AlsaAudioOutputStream::setVolume(float volume)
	{
		AudioOutputStream::setVolume(volume);
		AlsaAudioStream::setVolume(volume);
	}

	bool AlsaAudioOutputStream::getMute()
	{
		return AlsaAudioStream::getMute();
	}

	void AlsaAudioOutputStream::setMute(bool mute)
	{
		AudioOutputStream::setMute(mute);
		AlsaAudioStream::setMute(mute);
	}

	void AlsaAudioOutputStream::openInternal()
	{
		float volume = AudioOutputStream::getVolume();
		unsigned latency = AudioOutputStream::getBufferLatency();
		AudioFormat format = AudioOutputStream::getAudioFormat();

		AlsaAudioStream::open(SND_PCM_STREAM_CAPTURE, format, latency);

		// Calculate the buffer size for the specified stream latency.
		int ioSize = (format.getSampleRate() * format.getChannels() * (format.bitsPerSample() / 8) * latency) / 1000;
		unsigned frameSize = format.getChannels() * (format.bitsPerSample() / 8);

		initAudioBuffer(ioSize);
		
		initBuffer(periodSize * frameSize);
	}

	void AlsaAudioOutputStream::closeInternal()
	{
		AlsaAudioStream::close();
		
		freeAudioBuffer();
	}

	void AlsaAudioOutputStream::startInternal()
	{
		AlsaAudioStream::start();
	}

	void AlsaAudioOutputStream::stopInternal()
	{
		AlsaAudioStream::stop();
	}

	void AlsaAudioOutputStream::processAudio(snd_pcm_t * handle, snd_pcm_sframes_t * result)
	{
		AudioFormat format = getAudioFormat();
		unsigned frameSize = format.getChannels() * (format.bitsPerSample() / 8);
		
		printf("Read Audio: %d\n", periodSize);
		
		*result = snd_pcm_readi(handle, buffer.data(), periodSize);
		
		printf("Result: %d\n", *result);
		
		if (*result > 0) {
			writeAudio(buffer.data(), *result * frameSize);
		}
	}
}