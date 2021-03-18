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

#include "AVFTypeConverter.h"
#include "AVdevException.h"

namespace avdev
{
	const PictureControlMap AVFTypeConverter::getPictureControlMap()
	{
		return pictureControlMap;
	}
	
	const CameraControlMap AVFTypeConverter::getCameraControlMap()
	{
		return cameraControlMap;
	}
	
	const PixelFormat AVFTypeConverter::toPixelFormat(const std::uint32_t & apiFormat)
	{
		auto got = pixelFormatMap.find(apiFormat);
		
		if (got == pixelFormatMap.end()) {
			throw AVdevException("No mapping for pixel format: %d.", apiFormat);
		}
		
		PixelFormat format = got->second;
		
		return format;
	}
	
	const std::uint32_t AVFTypeConverter::toApiType(const PixelFormat & format)
	{
		auto got = std::find_if(pixelFormatMap.begin(), pixelFormatMap.end(),
								[format](const std::pair<const std::uint32_t, const PixelFormat> & vt) { return vt.second == format; });
		
		if (got == pixelFormatMap.end()) {
			throw AVdevException("No mapping for pixel format: %s.", PixelFormatToString(format).c_str());
		}
		
		return got->first;
	}
	
	const PictureControlType AVFTypeConverter::toPictureControlType(const std::uint32_t & apiType)
	{
		auto got = pictureControlMap.find(apiType);
		
		if (got == pictureControlMap.end()) {
			throw AVdevException("No mapping for picture control type: %d.", apiType);
		}
		
		return got->second;
	}
	
	const std::uint32_t AVFTypeConverter::toApiType(const PictureControlType & type)
	{
		auto got = std::find_if(pictureControlMap.begin(), pictureControlMap.end(),
								[type](const std::pair<const std::uint32_t, const PictureControlType> & vt) { return vt.second == type; });
		
		if (got == pictureControlMap.end()) {
			throw AVdevException("No mapping for picture control type: %d.", type);
		}
		
		return got->first;
	}
	
	const CameraControlType AVFTypeConverter::toCameraControlType(const std::uint32_t & apiType)
	{
		auto got = cameraControlMap.find(apiType);
		
		if (got == cameraControlMap.end()) {
			throw AVdevException("No mapping for camera control type: %d.", apiType);
		}
		
		return got->second;
	}
	
	const std::uint32_t AVFTypeConverter::toApiType(const CameraControlType & type)
	{
		auto got = std::find_if(cameraControlMap.begin(), cameraControlMap.end(),
								[type](const std::pair<const std::uint32_t, const CameraControlType> & vt) { return vt.second == type; });
		
		if (got == cameraControlMap.end()) {
			throw AVdevException("No mapping for camera control type: %d.", type);
		}
		
		return got->first;
	}
	
}