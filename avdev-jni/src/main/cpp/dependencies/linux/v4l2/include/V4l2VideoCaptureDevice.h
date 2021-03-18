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

#ifndef AVDEV_V4l2_VIDEO_CAPTURE_DEVICE_H_
#define AVDEV_V4l2_VIDEO_CAPTURE_DEVICE_H_

#include "avdev-v4l2.h"

#include "VideoCaptureDevice.h"

namespace avdev
{
	class V4l2VideoCaptureDevice : public VideoCaptureDevice
	{
		public:
			V4l2VideoCaptureDevice(std::string name, std::string descriptor);
			~V4l2VideoCaptureDevice();

			std::list<PictureFormat> getPictureFormats();
			std::list<PictureControl> getPictureControls();
			std::list<CameraControl> getCameraControls();

			void setPictureControlAutoMode(PictureControlType type, bool autoMode);
			bool getPictureControlAutoMode(PictureControlType type);
			void setPictureControlValue(PictureControlType type, long value);
			long getPictureControlValue(PictureControlType type);

			void setCameraControlAutoMode(CameraControlType type, bool autoMode);
			bool getCameraControlAutoMode(CameraControlType type);
			void setCameraControlValue(CameraControlType type, long value);
			long getCameraControlValue(CameraControlType type);

			void setPictureFormat(PictureFormat format);
			PictureFormat const& getPictureFormat() const;

			void setFrameRate(float frameRate);
			float getFrameRate() const;

			PVideoOutputStream createOutputStream(PVideoSink sink);

		private:
			int v4l2_fd;
	};
}

#endif