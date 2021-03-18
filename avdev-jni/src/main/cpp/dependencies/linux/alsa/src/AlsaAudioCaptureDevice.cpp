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

#include "AlsaAudioCaptureDevice.h"
#include "AlsaAudioOutputStream.h"

namespace avdev
{
	AlsaAudioCaptureDevice::AlsaAudioCaptureDevice(std::string name, std::string descriptor) :
		AudioCaptureDevice(name, descriptor)
	{
	}

	AlsaAudioCaptureDevice::~AlsaAudioCaptureDevice()
	{
	}

	PAudioOutputStream AlsaAudioCaptureDevice::createOutputStream(PAudioSink sink)
	{
		PAudioOutputStream stream = std::make_shared<AlsaAudioOutputStream>(getDescriptor(), sink);
		return stream;
	}
}