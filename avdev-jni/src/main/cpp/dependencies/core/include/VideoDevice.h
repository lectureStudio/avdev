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

#ifndef AVDEV_CORE_VIDEO_DEVICE_H_
#define AVDEV_CORE_VIDEO_DEVICE_H_

#include <list>
#include "Device.h"
#include "PictureFormat.h"

namespace avdev
{
	class VideoDevice : public Device
	{
		public:
			virtual ~VideoDevice() {};

			virtual std::list<PictureFormat> getPictureFormats() = 0;

			virtual void setPictureFormat(PictureFormat format) = 0;
			virtual PictureFormat const& getPictureFormat() const = 0;

			virtual void setFrameRate(float frameRate) = 0;
			virtual float getFrameRate() const = 0;

		protected:
			VideoDevice(std::string name, std::string descriptor);

			std::list<PictureFormat> formats;
			PictureFormat format;
			float frameRate;
	};
}

#endif