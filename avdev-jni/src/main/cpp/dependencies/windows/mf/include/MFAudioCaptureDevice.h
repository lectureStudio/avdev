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

#ifndef AVDEV_MF_AUDIO_CAPTURE_DEVICE_H_
#define AVDEV_MF_AUDIO_CAPTURE_DEVICE_H_

#include "AudioCaptureDevice.h"

namespace avdev
{
	class MFAudioCaptureDevice : public AudioCaptureDevice
	{
		public:
			MFAudioCaptureDevice(std::string name, std::string descriptor);
			~MFAudioCaptureDevice();

			PAudioOutputStream createOutputStream(PAudioSink sink);
	};
}

#endif