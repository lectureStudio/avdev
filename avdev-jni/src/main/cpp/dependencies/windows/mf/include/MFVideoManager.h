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

#ifndef AVDEV_MF_VIDEO_MANAGER_H_
#define AVDEV_MF_VIDEO_MANAGER_H_

#include "VideoManager.h"
#include "WinHotplugNotifier.h"
#include "MFVideoCaptureDevice.h"

namespace avdev
{
	class MFVideoManager : public VideoManager, WinHotplugNotifier
	{
		public:
			MFVideoManager();
			~MFVideoManager();

			std::set<PVideoCaptureDevice> getVideoCaptureDevices();

		protected:
			void onDeviceConnected(std::wstring symLink);
			void onDeviceDisconnected(std::wstring symLink);

		private:
			void enumerateDevices(std::wstring * symLink);
			std::shared_ptr<VideoDevice> createVideoDevice(WCHAR * symbolicLink, WCHAR * friendlyName);

			bool insertVideoDevice(std::shared_ptr<VideoDevice> device);
	};
}

#endif