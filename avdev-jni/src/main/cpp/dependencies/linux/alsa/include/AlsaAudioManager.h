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

#ifndef AVDEV_ALSA_AUDIO_MANAGER_H_
#define AVDEV_ALSA_AUDIO_MANAGER_H_

#include <alsa/asoundlib.h>

#include "AudioManager.h"
#include "AlsaAudioCaptureDevice.h"
#include "AlsaAudioPlaybackDevice.h"

namespace avdev
{
	class AlsaAudioManager : public AudioManager
	{
		public:
			static AlsaAudioManager & getInstance();
			~AlsaAudioManager();

			std::set<PAudioCaptureDevice> getAudioCaptureDevices();
			std::set<PAudioPlaybackDevice> getAudioPlaybackDevices();

		private:
			// Singleton restrictions.
			AlsaAudioManager();
			AlsaAudioManager(AlsaAudioManager const &) = delete;
			void operator=(AlsaAudioManager const &) = delete;

			void enumerateDevices(snd_pcm_stream_t stream);
			bool insertDevice(std::shared_ptr<AudioDevice> device, snd_pcm_stream_t stream);
	};
}

#endif