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

#include "PulseAudioPlaybackDevice.h"
#include "PulseAudioInputStream.h"

namespace avdev
{
	PulseAudioPlaybackDevice::PulseAudioPlaybackDevice(std::string name, std::string descriptor, uint32_t index) :
		AudioPlaybackDevice(name, descriptor),
		PulseAudioDevice(index)
	{
	}

	PulseAudioPlaybackDevice::~PulseAudioPlaybackDevice()
	{
	}
	
	PAudioInputStream PulseAudioPlaybackDevice::createInputStream(PAudioSource source)
	{
		return std::make_unique<PulseAudioInputStream>(getDescriptor(), source);
	}
}
