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

#ifndef AVDEV_CORE_AUDIO_CAPTURE_DEVICE_H_
#define AVDEV_CORE_AUDIO_CAPTURE_DEVICE_H_

#include "AudioDevice.h"
#include "AudioSink.h"
#include "AudioOutputStream.h"

#include <memory>

namespace avdev
{
	class AudioCaptureDevice : public AudioDevice
	{
		public:
			virtual ~AudioCaptureDevice() {};

			virtual PAudioOutputStream createOutputStream(PAudioSink sink) = 0;

		protected:
			AudioCaptureDevice(std::string name, std::string descriptor);
	};


	using PAudioCaptureDevice = std::shared_ptr<AudioCaptureDevice>;
}

#endif