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

#ifndef AVDEV_ALSA_AUDIO_STREAM_H_
#define AVDEV_ALSA_AUDIO_STREAM_H_

#include <alsa/asoundlib.h>

#include <chrono>
#include <string>

#include "AudioFormat.h"
#include "Thread.h"

namespace avdev
{
	enum class DumpContext
	{
		// PCM info. 
		PCM_INFO,
		// Current hardware setup for PCM.
		PCM_HW_SETUP,
		// Current software setup for PCM.
		PCM_SW_SETUP,
		// Current hardware and software setup for PCM.
		PCM_SETUP,
		// PCM hardware configuration.
		PCM_HW_PARAMS,
		// PCM software configuration.
		PCM_SW_PARAMS,
		// Status.
		STATUS
	};
	
	class AlsaAudioStream : Thread
	{
		public:
			AlsaAudioStream(std::string deviceId);
			virtual ~AlsaAudioStream();

			float getVolume();
			void setVolume(float volume);

			bool getMute();
			void setMute(bool mute);

		protected:
			std::string deviceId;
			
			snd_pcm_t * handle;
			snd_pcm_uframes_t periodSize;
			
			void open(snd_pcm_stream_t stream, AudioFormat & format, unsigned latency);
			void close();
			void start();
			void stop();
			
			void run();
			
			virtual void processAudio(snd_pcm_t * handle, snd_pcm_sframes_t * result) = 0;
			
			template <typename ...Args>
			void throwOnError(int error, const char * message, Args && ...args);

		private:
			void setHardwareParameters(AudioFormat & format, unsigned latency);
			void setSoftwareParameters(AudioFormat & format, unsigned latency);
			void dump(DumpContext contextType, void * context);
			bool recovery(int error);
	};
}

#endif