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

#ifndef AVDEV_AVF_TYPE_CONVERTER_H_
#define AVDEV_AVF_TYPE_CONVERTER_H_

#include "PictureFormat.h"
#include "PictureControl.h"
#include "CameraControl.h"

#include <cstdint>
#include <unordered_map>

#include <AVFoundation/AVFoundation.h>

namespace avdev
{
	struct UINTHasher
	{
		std::size_t operator()(const std::uint32_t & id) const
		{
			return std::hash<unsigned long>()(id);
		}
	};
	
	using PixelFormatMap = std::unordered_map<const std::uint32_t, const PixelFormat, UINTHasher>;
	using CameraControlMap = std::unordered_map<const std::uint32_t, const CameraControlType, UINTHasher>;
	using PictureControlMap = std::unordered_map<const std::uint32_t, const PictureControlType, UINTHasher>;
	
	static const PixelFormatMap pixelFormatMap =
	{
		/* Uncompressed RGB Formats */
		{ kCVPixelFormatType_8Indexed, avdev::PixelFormat::GREY },
		{ kCVPixelFormatType_16LE555,  avdev::PixelFormat::RGB555 },
		{ kCVPixelFormatType_16LE565,  avdev::PixelFormat::RGB565 },
		{ kCVPixelFormatType_24BGR,    avdev::PixelFormat::BGR24 },
		{ kCVPixelFormatType_24RGB,    avdev::PixelFormat::RGB24 },
		{ kCVPixelFormatType_32BGRA,   avdev::PixelFormat::BGR32 },
		{ kCVPixelFormatType_32RGBA,   avdev::PixelFormat::RGB32 },
		{ kCVPixelFormatType_32ARGB,   avdev::PixelFormat::ARGB },
		
		/* YUV Formats */
		{ kCVPixelFormatType_422YpCbCr8,                   avdev::PixelFormat::UYVY },
		{ kCVPixelFormatType_420YpCbCr8Planar,             avdev::PixelFormat::I420 },
		{ kCVPixelFormatType_422YpCbCr8_yuvs,              avdev::PixelFormat::YUYV },
		{ kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange, avdev::PixelFormat::NV12 },
		
		/* Encoded Video Types */
		{ kCMVideoCodecType_JPEG,          avdev::PixelFormat::JPEG },
		{ kCMVideoCodecType_MPEG4Video,    avdev::PixelFormat::MPEG },
		{ kCMVideoCodecType_JPEG_OpenDML,  avdev::PixelFormat::MJPG }
	};
	
	static const PictureControlMap pictureControlMap =
	{
		
	};
	
	static const CameraControlMap cameraControlMap =
	{
		
	};
	
	
	class AVFTypeConverter
	{
		public:
			static const PictureControlMap getPictureControlMap();
			static const CameraControlMap getCameraControlMap();

			static const PixelFormat toPixelFormat(const std::uint32_t & apiFormat);
		
			static const std::uint32_t toApiType(const PixelFormat & format);
		
			static const PictureControlType toPictureControlType(const std::uint32_t & apiType);
		
			static const std::uint32_t toApiType(const PictureControlType & type);
		
			static const CameraControlType toCameraControlType(const std::uint32_t & apiFormat);
		
			static const std::uint32_t toApiType(const CameraControlType & type);
	};
}

#endif