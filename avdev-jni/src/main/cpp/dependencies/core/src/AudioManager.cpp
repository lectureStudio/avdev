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

#include "AudioManager.h"

namespace avdev
{
	AudioManager::AudioManager() :
		defaultCapture(nullptr),
		defaultPlayback(nullptr)
	{
	}
	
	PAudioCaptureDevice AudioManager::getDefaultAudioCaptureDevice()
	{
		if (getDefaultCaptureDevice() == nullptr && captureDevices.empty()) {
			getAudioCaptureDevices();
		}
		
		if (getDefaultCaptureDevice() == nullptr && !captureDevices.empty()) {
			setDefaultCaptureDevice(*captureDevices.devices().begin());
		}
		
		return getDefaultCaptureDevice();
	}
	
	PAudioPlaybackDevice AudioManager::getDefaultAudioPlaybackDevice()
	{
		if (getDefaultPlaybackDevice() == nullptr && playbackDevices.empty()) {
			getAudioPlaybackDevices();
		}
		
		if (getDefaultPlaybackDevice() == nullptr && !playbackDevices.empty()) {
			setDefaultPlaybackDevice(*playbackDevices.devices().begin());
		}
		
		return getDefaultPlaybackDevice();
	}
	
	void AudioManager::setDefaultCaptureDevice(PAudioCaptureDevice device)
	{
		std::unique_lock<std::mutex> mlock(mutex);
		
		defaultCapture = device;
	}
	
	PAudioCaptureDevice AudioManager::getDefaultCaptureDevice()
	{
		std::unique_lock<std::mutex> mlock(mutex);
		
		return defaultCapture;
	}
	
	void AudioManager::setDefaultPlaybackDevice(PAudioPlaybackDevice device)
	{
		std::unique_lock<std::mutex> mlock(mutex);
		
		defaultPlayback = device;
	}
	
	PAudioPlaybackDevice AudioManager::getDefaultPlaybackDevice()
	{
		std::unique_lock<std::mutex> mlock(mutex);
		
		return defaultPlayback;
	}
}