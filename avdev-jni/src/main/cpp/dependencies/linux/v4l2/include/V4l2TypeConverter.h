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

#ifndef AVDEV_V4l2_TYPE_CONVERTER_H_
#define AVDEV_V4l2_TYPE_CONVERTER_H_

#include "avdev-v4l2.h"

#include "PictureFormat.h"
#include "PictureControl.h"
#include "CameraControl.h"

#include <algorithm>
#include <unordered_map>

namespace avdev
{
	namespace V4l2TypeConverter
	{
		struct UIntHasher
		{
			std::size_t operator()(const std::uint32_t & id) const
			{
				std::size_t hash = std::hash<std::uint32_t>()(id);

				return hash;
			}
		};
		
		using PictureControlMap = std::unordered_map<const std::uint32_t, const PictureControlType, UIntHasher>;
		using CameraControlMap = std::unordered_map<const std::uint32_t, const CameraControlType, UIntHasher>;
		using PixelFormatMap = std::unordered_map<const std::uint32_t, const PixelFormat, UIntHasher>;
		
		
		const PixelFormat toPixelFormat(const std::uint32_t & apiformat);
			
		const std::uint32_t toApiType(const PixelFormat & format);
			
		const PictureControlType toPictureControlType(const std::uint32_t & apiType);
			
		const std::uint32_t toApiType(const PictureControlType & type);
		
		const CameraControlType toCameraControlType(const std::uint32_t & apiType);
		
		const std::uint32_t toApiType(const CameraControlType & type);

	}
}

#endif