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

#include "AVFVideoCaptureDevice.h"
#include "AVFVideoOutputStream.h"
#include "AVFTypeConverter.h"

#include <AVFoundation/AVFoundation.h>

namespace avdev
{
	AVFVideoCaptureDevice::AVFVideoCaptureDevice(std::string name, std::string descriptor)
		: VideoCaptureDevice(name, descriptor)
	{
		
	}

	AVFVideoCaptureDevice::~AVFVideoCaptureDevice()
	{
		
	}

	std::list<PictureFormat> AVFVideoCaptureDevice::getPictureFormats()
	{
		if (formats.empty()) {
			NSString * deviceID = [NSString stringWithUTF8String: getDescriptor().c_str()];
			AVCaptureDevice * device = [AVCaptureDevice deviceWithUniqueID: deviceID];
			NSArray * devFormats = [device formats];
			
			for (AVCaptureDeviceFormat * format in devFormats) {
				const CMVideoDimensions dimension = CMVideoFormatDescriptionGetDimensions(format.formatDescription);
				const FourCharCode fcc = CMFormatDescriptionGetMediaSubType(format.formatDescription);
				
				AVCaptureDeviceFormat * bestFormat = nil;
				AVFrameRateRange * bestFrameRateRange = nil;
				
				for (AVFrameRateRange * range in format.videoSupportedFrameRateRanges) {
					if (range.maxFrameRate > bestFrameRateRange.maxFrameRate) {
						bestFormat = format;
						bestFrameRateRange = range;
					}
				}

				formats.push_back(PictureFormat(dimension.width, dimension.height, AVFTypeConverter::toPixelFormat(fcc)));
			}
			
			formats.unique();
		}
		
		return formats;
	}
	
	std::list<PictureControl> AVFVideoCaptureDevice::getPictureControls()
	{
		std::list<PictureControl> controls;

		
		
		return controls;
	}

	std::list<CameraControl> AVFVideoCaptureDevice::getCameraControls()
	{
		std::list<CameraControl> controls;

		
		
		return controls;
	}

	void AVFVideoCaptureDevice::setPictureControlAutoMode(PictureControlType type, bool autoMode)
	{
		
	}
	
	bool AVFVideoCaptureDevice::getPictureControlAutoMode(PictureControlType type)
	{
		
		return false;
	}

	void AVFVideoCaptureDevice::setPictureControlValue(PictureControlType type, long value)
	{
		
	}
	
	long AVFVideoCaptureDevice::getPictureControlValue(PictureControlType type)
	{
		
		return 0;
	}

	void AVFVideoCaptureDevice::setCameraControlAutoMode(CameraControlType type, bool autoMode)
	{
		
	}
	
	bool AVFVideoCaptureDevice::getCameraControlAutoMode(CameraControlType type)
	{
		return false;
	}

	void AVFVideoCaptureDevice::setCameraControlValue(CameraControlType type, long value)
	{
		
	}
	
	long AVFVideoCaptureDevice::getCameraControlValue(CameraControlType type)
	{
		return 0;
	}
	
	void AVFVideoCaptureDevice::setPictureFormat(PictureFormat format)
	{
		this->format = format;
	}
	
	PictureFormat const& AVFVideoCaptureDevice::getPictureFormat() const
	{
		return format;
	}

	void AVFVideoCaptureDevice::setFrameRate(float frameRate)
	{
		this->frameRate = frameRate;
	}
	
	float AVFVideoCaptureDevice::getFrameRate() const
	{
		return frameRate;
	}
	
	PVideoOutputStream AVFVideoCaptureDevice::createOutputStream(PVideoSink sink)
	{
		PVideoOutputStream stream = std::make_unique<AVFVideoOutputStream>(getDescriptor(), sink);
		stream->setFrameRate(getFrameRate());
		stream->setPictureFormat(getPictureFormat());
		
		return stream;
	}

}