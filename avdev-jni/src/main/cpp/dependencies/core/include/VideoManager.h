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

#ifndef AVDEV_CORE_VIDEO_MANAGER_H_
#define AVDEV_CORE_VIDEO_MANAGER_H_

#include "DeviceList.h"
#include "DeviceManager.h"
#include "VideoCaptureDevice.h"

#include <mutex>
#include <set>

namespace avdev
{
	using PVideoCaptureDevice = std::shared_ptr<VideoCaptureDevice>;


	class VideoManager : public DeviceManager
	{
		public:
			VideoManager();
			virtual ~VideoManager() {};

			PVideoCaptureDevice getDefaultVideoCaptureDevice();

			virtual std::set<PVideoCaptureDevice> getVideoCaptureDevices() = 0;

		protected:
			void setDefaultCaptureDevice(PVideoCaptureDevice device);
			PVideoCaptureDevice getDefaultCaptureDevice();
		
			DeviceList<PVideoCaptureDevice> captureDevices;

		private:
			PVideoCaptureDevice defaultCapture;
		
			std::mutex mutex;
	};
}

#endif