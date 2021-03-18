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

#include "AlsaAudioInputStream.h"
#include "Log.h"

namespace avdev
{
	AlsaAudioInputStream::AlsaAudioInputStream(std::string deviceId, PAudioSource source) :
		AlsaAudioStream(deviceId),
		AudioInputStream(source)
	{
	}

	float AlsaAudioInputStream::getVolume()
	{
		return AlsaAudioStream::getVolume();
	}

	void AlsaAudioInputStream::setVolume(float volume)
	{
		AudioInputStream::setVolume(volume);
		AlsaAudioStream::setVolume(volume);
	}

	bool AlsaAudioInputStream::getMute()
	{
		return AlsaAudioStream::getMute();
	}

	void AlsaAudioInputStream::setMute(bool mute)
	{
		AudioInputStream::setMute(mute);
		AlsaAudioStream::setMute(mute);
	}

	void AlsaAudioInputStream::openInternal()
	{
		float volume = AudioInputStream::getVolume();
		unsigned latency = AudioInputStream::getBufferLatency();
		AudioFormat format = AudioInputStream::getAudioFormat();

		AlsaAudioStream::open(SND_PCM_STREAM_PLAYBACK, format, latency);

		initAudioBuffer(AudioInputStream::getPlaybackBufferSize());
		
		unsigned frameSize = format.getChannels() * (format.bitsPerSample() / 8);
		initBuffer(periodSize * frameSize);
	}

	void AlsaAudioInputStream::closeInternal()
	{
		AlsaAudioStream::close();

		freeAudioBuffer();
	}

	void AlsaAudioInputStream::startInternal()
	{
		AlsaAudioStream::start();
	}

	void AlsaAudioInputStream::stopInternal()
	{
		AlsaAudioStream::stop();
	}
	
	void AlsaAudioInputStream::processAudio(snd_pcm_t * handle, snd_pcm_sframes_t * result)
	{
		printf("Write Audio: %d\n", periodSize);
		
		AudioFormat format = getAudioFormat();
		unsigned frameSize = format.getChannels() * (format.bitsPerSample() / 8);
		unsigned framesBytes = periodSize * frameSize;
		
		int read = readAudio(framesBytes);
		if (read < 1) {
			*result = read;
			return;
		}
		
		*result = snd_pcm_writei(handle, buffer.data(), periodSize);
		
		printf("Result: %d\n", *result);
	}

}