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

#ifndef AVDEV_V4l2_VIDEO_MANAGER_H_
#define AVDEV_V4l2_VIDEO_MANAGER_H_

#include "avdev-v4l2.h"

#include <libudev.h>
#include "Thread.h"
#include "VideoManager.h"

#define UDEV_SUBSYSTEM "video4linux"
#define UDEV_ADD "add"
#define UDEV_REMOVE "remove"
#define UDEV_CHANGE "change"

namespace avdev
{
	class V4l2VideoManager : public VideoManager, public Thread
	{
		public:
			V4l2VideoManager();
			~V4l2VideoManager();

			std::set<PVideoCaptureDevice> getVideoCaptureDevices();

		protected:
			void run();
			void addDevice(const char * name, const char * descriptor);
			void removeDevice(const char * name, const char * descriptor);

		private:
			struct udev * udev;
	};
}

#endif
