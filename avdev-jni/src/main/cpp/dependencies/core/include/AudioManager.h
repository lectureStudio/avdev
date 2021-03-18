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

#ifndef AVDEV_CORE_AUDIO_MANAGER_H_
#define AVDEV_CORE_AUDIO_MANAGER_H_

#include "DeviceList.h"
#include "DeviceManager.h"
#include "AudioCaptureDevice.h"
#include "AudioPlaybackDevice.h"

#include <set>

namespace avdev
{
	class AudioManager : public DeviceManager
	{
		public:
			AudioManager();
			virtual ~AudioManager() {};

			PAudioCaptureDevice getDefaultAudioCaptureDevice();
			PAudioPlaybackDevice getDefaultAudioPlaybackDevice();

			virtual std::set<PAudioCaptureDevice> getAudioCaptureDevices() = 0;
			virtual std::set<PAudioPlaybackDevice> getAudioPlaybackDevices() = 0;

		protected:
			void setDefaultCaptureDevice(PAudioCaptureDevice device);
			PAudioCaptureDevice getDefaultCaptureDevice();

			void setDefaultPlaybackDevice(PAudioPlaybackDevice device);
			PAudioPlaybackDevice getDefaultPlaybackDevice();

			DeviceList<PAudioCaptureDevice> captureDevices;
			DeviceList<PAudioPlaybackDevice> playbackDevices;

		private:
			PAudioCaptureDevice defaultCapture;
			PAudioPlaybackDevice defaultPlayback;

			std::mutex mutex;
	};
}

#endif