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

#ifndef AVDEV_PULSE_AUDIO_MANAGER_H_
#define AVDEV_PULSE_AUDIO_MANAGER_H_

#include <pulse/pulseaudio.h>

#include "AudioManager.h"
#include "PulseAudioCaptureDevice.h"
#include "PulseAudioPlaybackDevice.h"

namespace avdev
{
	class PulseAudioManager : public AudioManager
	{
		public:
			PulseAudioManager();
			~PulseAudioManager();

			PAudioCaptureDevice getDefaultAudioCaptureDevice();
			PAudioPlaybackDevice getDefaultAudioPlaybackDevice();

			std::set<PAudioCaptureDevice> getAudioCaptureDevices();
			std::set<PAudioPlaybackDevice> getAudioPlaybackDevices();

		private:
			void dispose();

			void iterate(pa_threaded_mainloop * main_loop, pa_operation * op);

			/* PulseAudio API callbacks. */
			static void stateCallback(pa_context * ctx, void * userdata);
			static void serverInfoCallback(pa_context * ctx, const pa_server_info * info, void * userdata);
			static void subscribeCallback(pa_context * ctx, pa_subscription_event_type_t t, uint32_t idx, void * userdata);
			static void getSourceCallback(pa_context * ctx, const pa_source_info * info, int last, void * userdata);
            static void newSourceCallback(pa_context * ctx, const pa_source_info * info, int last, void * userdata);
			static void getSinkCallback(pa_context * ctx, const pa_sink_info * info, int last, void * userdata);
			static void newSinkCallback(pa_context * ctx, const pa_sink_info * info, int last, void * userdata);

			template<typename T, typename S>
			void insertDevice(DeviceList<T> & devices, const char * name, const char * desc, uint32_t index, bool notify);

			template<typename T, typename S>
			void removeDevice(DeviceList<T> & devices, uint32_t index);

			pa_threaded_mainloop * mainloop;
			pa_context * context;

			std::string defaultCaptureName;
			std::string defaultPlaybackName;
	};
}

#endif
