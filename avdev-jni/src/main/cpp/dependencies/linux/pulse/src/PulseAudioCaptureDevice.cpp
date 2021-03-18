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

#include "PulseAudioCaptureDevice.h"
#include "PulseAudioOutputStream.h"

namespace avdev
{
	PulseAudioCaptureDevice::PulseAudioCaptureDevice(std::string name, std::string descriptor, uint32_t index) :
		AudioCaptureDevice(name, descriptor),
		PulseAudioDevice(index)
	{
	}

	PulseAudioCaptureDevice::~PulseAudioCaptureDevice()
	{
	}

	PAudioOutputStream PulseAudioCaptureDevice::createOutputStream(PAudioSink sink)
	{
		return std::make_unique<PulseAudioOutputStream>(getDescriptor(), sink);
	}
}
