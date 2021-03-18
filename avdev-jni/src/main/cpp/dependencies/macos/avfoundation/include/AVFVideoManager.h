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

#ifndef AVDEV_AVF_VIDEO_MANAGER_H_
#define AVDEV_AVF_VIDEO_MANAGER_H_

#include <AVFoundation/AVFoundation.h>

#include "VideoManager.h"

namespace avdev
{
	class AVFVideoManager : public VideoManager
	{
		public:
			AVFVideoManager();
			~AVFVideoManager();

			std::set<PVideoCaptureDevice> getVideoCaptureDevices();

		private:
			void insertDevice(AVCaptureDevice * device, bool notify);
			void removeDevice(AVCaptureDevice * device, bool notify);

			void onDeviceConnected(AVCaptureDevice * device);
			void onDeviceDisconnected(AVCaptureDevice * device);

		private:
			id devConnectObserver;
			id devDisconnectObserver;
	};
}

#endif