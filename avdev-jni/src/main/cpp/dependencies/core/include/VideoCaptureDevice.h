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

#ifndef AVDEV_CORE_VIDEO_CAPTURE_DEVICE_H_
#define AVDEV_CORE_VIDEO_CAPTURE_DEVICE_H_

#include <memory>
#include "VideoDevice.h"
#include "VideoSink.h"
#include "VideoOutputStream.h"
#include "CameraControl.h"
#include "PictureControl.h"

namespace avdev
{
	class VideoCaptureDevice : public VideoDevice
	{
		public:
			virtual ~VideoCaptureDevice() {};

			virtual PVideoOutputStream createOutputStream(PVideoSink sink) = 0;

			virtual std::list<PictureControl> getPictureControls() = 0;
			virtual std::list<CameraControl> getCameraControls() = 0;

			virtual void setPictureControlAutoMode(PictureControlType type, bool autoMode) = 0;
			virtual bool getPictureControlAutoMode(PictureControlType type) = 0;
			virtual void setPictureControlValue(PictureControlType type, long value) = 0;
			virtual long getPictureControlValue(PictureControlType type) = 0;

			virtual void setCameraControlAutoMode(CameraControlType type, bool autoMode) = 0;
			virtual bool getCameraControlAutoMode(CameraControlType type) = 0;
			virtual void setCameraControlValue(CameraControlType type, long value) = 0;
			virtual long getCameraControlValue(CameraControlType type) = 0;

		protected:
			VideoCaptureDevice(std::string name, std::string descriptor);

			std::list<PictureControl> pictureControls;
			std::list<CameraControl> cameraControls;
	};
}

#endif