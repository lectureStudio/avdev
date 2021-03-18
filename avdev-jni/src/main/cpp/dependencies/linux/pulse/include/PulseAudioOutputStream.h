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

#ifndef AVDEV_PULSE_AUDIO_OUTPUT_STREAM_H_
#define AVDEV_PULSE_AUDIO_OUTPUT_STREAM_H_

#include "AudioOutputStream.h"
#include "PulseAudioStream.h"

namespace avdev
{
	class PulseAudioOutputStream : public PulseAudioStream, public AudioOutputStream
	{
		public:
			PulseAudioOutputStream(std::string name, PAudioSink sink);
			virtual ~PulseAudioOutputStream() {};

			float getVolume();
			void setVolume(float volume);

			bool getMute();
			void setMute(bool mute);

		protected:
			void openInternal();
			void closeInternal();
			void startInternal();
			void stopInternal();
			
			static void streamReadCallback(pa_stream * paStream, size_t length, void * userdata);
	};
}

#endif