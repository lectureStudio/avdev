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

#ifndef AVDEV_MF_TYPE_CONVERTER_H_
#define AVDEV_MF_TYPE_CONVERTER_H_

#include "AudioFormat.h"
#include "PictureFormat.h"
#include "PictureControl.h"
#include "CameraControl.h"

#include <unordered_map>
#include <Strmif.h>
#include <Ks.h>
#include <Ksmedia.h>

namespace avdev
{
	namespace MFTypeConverter
	{
		struct GUIDHasher
		{
			std::size_t operator()(const GUID & guid) const
			{
				std::string d4;
				d4 += guid.Data4[0];
				d4 += guid.Data4[1];
				d4 += guid.Data4[2];
				d4 += guid.Data4[3];
				d4 += guid.Data4[4];
				d4 += guid.Data4[5];
				d4 += guid.Data4[6];
				d4 += guid.Data4[7];

				std::size_t h1 = std::hash<unsigned long>()(guid.Data1);
				std::size_t h2 = std::hash<unsigned short>()(guid.Data2);
				std::size_t h3 = std::hash<unsigned short>()(guid.Data3);
				std::size_t h4 = std::hash<std::string>()(d4);

				return h1 ^ (h2 << 1) ^ (h3 << 1) ^ (h4 << 1);
			}
		};

		using PictureControlMap = std::unordered_map<KSPROPERTY_VIDCAP_VIDEOPROCAMP, PictureControlType>;
		using CameraControlMap = std::unordered_map<KSPROPERTY_VIDCAP_CAMERACONTROL, CameraControlType>;
		using PixelFormatMap = std::unordered_map<const GUID, const PixelFormat, GUIDHasher>;


		const PictureControlMap getPictureControlMap();
		const CameraControlMap getCameraControlMap();

		const SampleFormat toSampleFormat(const GUID & apiFormat, UINT32 bitsPerSample);

		const GUID toApiType(const SampleFormat & format);

		const PixelFormat toPixelFormat(const GUID & apiFormat);

		const GUID toApiType(const PixelFormat & format);

		const PictureControlType toPictureControlType(const KSPROPERTY_VIDCAP_VIDEOPROCAMP & apiType);

		const KSPROPERTY_VIDCAP_VIDEOPROCAMP toApiType(const PictureControlType & type);

		const CameraControlType toCameraControlType(const KSPROPERTY_VIDCAP_CAMERACONTROL & apiFormat);

		const KSPROPERTY_VIDCAP_CAMERACONTROL toApiType(const CameraControlType & type);

	}
}

#endif