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

#ifndef AVDEV_ALSA_AUDIO_OUTPUT_STREAM_H_
#define AVDEV_ALSA_AUDIO_OUTPUT_STREAM_H_

#include "AudioOutputStream.h"
#include "AlsaAudioStream.h"

namespace avdev
{
	class AlsaAudioOutputStream : public AlsaAudioStream, public AudioOutputStream
	{
		public:
			AlsaAudioOutputStream(std::string deviceId, PAudioSink sink);
			virtual ~AlsaAudioOutputStream() {};

			float getVolume();
			void setVolume(float volume);

			bool getMute();
			void setMute(bool mute);

		protected:
			void openInternal();
			void closeInternal();
			void startInternal();
			void stopInternal();
			
			void processAudio(snd_pcm_t * handle, snd_pcm_sframes_t * result);
	};
}

#endif