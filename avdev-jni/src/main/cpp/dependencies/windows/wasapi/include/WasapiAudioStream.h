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

#ifndef AVDEV_WASAPI_AUDIO_STREAM_H_
#define AVDEV_WASAPI_AUDIO_STREAM_H_

#include <windows.h>
#include <Avrt.h>
#include <Mftransform.h>
#include <Wmcodecdsp.h>
#include <Audioclient.h>
#include <Audiopolicy.h>
#include <Mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include "MFAudioResampler.h"
#include "WindowsHelper.h"
#include "WasapiAudioSessionEvents.h"
#include <chrono>

namespace avdev
{
	class WasapiAudioStream
	{
		public:
			WasapiAudioStream(std::wstring deviceId);
			virtual ~WasapiAudioStream();

			float getVolume();
			void setVolume(float volume);

			bool getMute();
			void setMute(bool mute);

			float getBufferLatency();

		protected:
			void initAudioClient(EDataFlow dataFlow, DWORD flags, AudioFormat format, unsigned latency, AudioSessionListener * listener);
			void shutdown();
			void start();
			void stop();
			virtual void processAudio() = 0;

			const std::wstring deviceId;

			HANDLE threadHandle;
			HANDLE threadShutdownEvent;
			HANDLE samplesReadyEvent;

			jni::ComPtr<IMMDevice> device;
			jni::ComPtr<IAudioClient> audioClient;
			jni::ComPtr<ISimpleAudioVolume> audioVolume;
			jni::ComPtr<IAudioStreamVolume> streamVolume;
			jni::ComPtr<IAudioSessionControl> sessionControl;

			std::unique_ptr<WasapiAudioSessionEvents> sessionEvents;

			MFAudioResampler * resampler;

		private:
			float lastVolume;

			UINT32 GetChannelCount();
			void setVolumeAllChannels(float volume);

			static DWORD WINAPI run(LPVOID context);
	};
}

#endif