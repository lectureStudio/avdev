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

#ifndef AVDEV_PULSE_AUDIO_STREAM_H_
#define AVDEV_PULSE_AUDIO_STREAM_H_

#include <pulse/pulseaudio.h>

#include <string>

#include "AudioDevice.h"
#include "AudioFormat.h"

namespace avdev
{
	class PulseAudioStream
	{
		public:
			PulseAudioStream(std::string name);
			virtual ~PulseAudioStream();

			float getVolume();
			bool getMute();

		protected:
			void close();
			void stop();
			
			void dispose();

			pa_sample_spec audioFormatToSampleSpec(const AudioFormat & format);
			void createContext(const char * contextName);
			bool isContextReady(pa_context * context, pa_threaded_mainloop * mainloop);
			bool isStreamReady(pa_stream * stream, pa_threaded_mainloop * mainloop);
			void completeOperation(pa_threaded_mainloop * mainloop, pa_operation * operation);
			void pauseStream(bool pause);
			
			static void contextStateCallback(pa_context * context, void * userdata);
			static void contextVolumeCallback(pa_context * context, const pa_source_info * info, int error, void * userdata);
			static void streamStateCallback(pa_stream * paStream, void * userdata);
			static void streamSuccessCallback(pa_stream * paStream, int success, void * userdata);
			
			std::string name;
			
			pa_threaded_mainloop * mainloop;
			pa_context * context;
			pa_stream * stream;
	};
}

#endif