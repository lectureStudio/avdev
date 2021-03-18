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

#ifndef AVDEV_PULSE_AUDIO_DEVICE_H_
#define AVDEV_PULSE_AUDIO_DEVICE_H_

#include <stdint.h>

namespace avdev
{
	class PulseAudioDevice
	{
		public:
			PulseAudioDevice(uint32_t index);
			virtual ~PulseAudioDevice();
			
			uint32_t getIndex() const;
			
		private:
			uint32_t index;
	};
}

#endif