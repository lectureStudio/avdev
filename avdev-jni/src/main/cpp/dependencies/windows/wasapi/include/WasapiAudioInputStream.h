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

#ifndef AVDEV_WASAPI_AUDIO_INPUT_STREAM_H_
#define AVDEV_WASAPI_AUDIO_INPUT_STREAM_H_

#include "AudioInputStream.h"
#include "AudioSessionListener.h"
#include "WasapiAudioStream.h"

namespace avdev
{
	class WasapiAudioInputStream : public WasapiAudioStream, public AudioInputStream, public AudioSessionListener
	{
		public:
			WasapiAudioInputStream(std::wstring deviceId, PAudioSource source);
			virtual ~WasapiAudioInputStream() {};
			
			float getVolume();
			void setVolume(float volume);

			bool getMute();
			void setMute(bool mute);

			void volumeChanged(float volume, bool mute) override;

		protected:
			void openInternal();
			void closeInternal();
			void startInternal();
			void stopInternal();

			void processAudio();

		private:
			jni::ComPtr<IAudioRenderClient> renderClient;
	};
}

#endif