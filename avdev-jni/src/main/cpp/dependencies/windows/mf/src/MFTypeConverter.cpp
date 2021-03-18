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

#include "MFTypeConverter.h"
#include "Exception.h"
#include "WindowsHelper.h"

#include <algorithm>
#include <functional>
#include <unordered_map>

namespace avdev
{
	namespace MFTypeConverter
	{
		PictureControlMap pictureControlMap =
		{
			{ KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS,               PictureControlType::Brightness },
			{ KSPROPERTY_VIDEOPROCAMP_CONTRAST,                 PictureControlType::Contrast },
			{ KSPROPERTY_VIDEOPROCAMP_HUE,                      PictureControlType::Hue },
			{ KSPROPERTY_VIDEOPROCAMP_SATURATION,               PictureControlType::Saturation },
			{ KSPROPERTY_VIDEOPROCAMP_SHARPNESS,                PictureControlType::Sharpness },
			{ KSPROPERTY_VIDEOPROCAMP_GAMMA,                    PictureControlType::Gamma },
			{ KSPROPERTY_VIDEOPROCAMP_COLORENABLE,              PictureControlType::ColorEnable },
			{ KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE,             PictureControlType::WhiteBalance },
			{ KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION,   PictureControlType::BacklightCompensation },
			{ KSPROPERTY_VIDEOPROCAMP_GAIN,                     PictureControlType::Gain },
			{ KSPROPERTY_VIDEOPROCAMP_DIGITAL_MULTIPLIER,       PictureControlType::DigitalMultiplier },
			{ KSPROPERTY_VIDEOPROCAMP_DIGITAL_MULTIPLIER_LIMIT, PictureControlType::DigitalMultiplierLimit },
			{ KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE_COMPONENT,   PictureControlType::WhiteBalanceComponent },
			{ KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY,      PictureControlType::PowerLineFrequency }
		};
		
		CameraControlMap cameraControlMap =
		{
			{ KSPROPERTY_CAMERACONTROL_PAN,                    CameraControlType::Pan },
			{ KSPROPERTY_CAMERACONTROL_TILT,                   CameraControlType::Tilt },
			{ KSPROPERTY_CAMERACONTROL_ROLL,                   CameraControlType::Roll },
			{ KSPROPERTY_CAMERACONTROL_ZOOM,                   CameraControlType::Zoom },
			{ KSPROPERTY_CAMERACONTROL_EXPOSURE,               CameraControlType::Exposure },
			{ KSPROPERTY_CAMERACONTROL_IRIS,                   CameraControlType::Iris },
			{ KSPROPERTY_CAMERACONTROL_FOCUS,                  CameraControlType::Focus },
			{ KSPROPERTY_CAMERACONTROL_SCANMODE,               CameraControlType::ScanMode },
			{ KSPROPERTY_CAMERACONTROL_PRIVACY,                CameraControlType::Privacy },
			{ KSPROPERTY_CAMERACONTROL_PANTILT,                CameraControlType::PanTilt },
			{ KSPROPERTY_CAMERACONTROL_PAN_RELATIVE,           CameraControlType::PanRelative },
			{ KSPROPERTY_CAMERACONTROL_TILT_RELATIVE,          CameraControlType::TiltRelative },
			{ KSPROPERTY_CAMERACONTROL_ROLL_RELATIVE,          CameraControlType::RollRelative },
			{ KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE,          CameraControlType::ZoomRelative },
			{ KSPROPERTY_CAMERACONTROL_EXPOSURE_RELATIVE,      CameraControlType::ExposureRelative },
			{ KSPROPERTY_CAMERACONTROL_IRIS_RELATIVE,          CameraControlType::IrisRelative },
			{ KSPROPERTY_CAMERACONTROL_FOCUS_RELATIVE,         CameraControlType::FocusRelative },
			{ KSPROPERTY_CAMERACONTROL_PANTILT_RELATIVE,       CameraControlType::PanTiltRelative },
			{ KSPROPERTY_CAMERACONTROL_FOCAL_LENGTH,           CameraControlType::FocalLength },
			{ KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY, CameraControlType::AutoExposurePriority }
		};

		PixelFormatMap pixelFormatMap =
		{
			/* Uncompressed RGB Formats */
			{ MFVideoFormat_ARGB32, avdev::PixelFormat::ARGB },
			{ MFVideoFormat_RGB32,  avdev::PixelFormat::RGB32 },
			{ MFVideoFormat_RGB24,  avdev::PixelFormat::RGB24 },
			{ MFVideoFormat_RGB555, avdev::PixelFormat::RGB555 },
			{ MFVideoFormat_RGB565, avdev::PixelFormat::RGB565 },
			{ MFVideoFormat_RGB8,   avdev::PixelFormat::GREY },

			/* YUV Formats: 8-Bit and Palettized */
			{ MFVideoFormat_AI44, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_AYUV, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_I420, avdev::PixelFormat::I420 },
			{ MFVideoFormat_IYUV, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_NV11, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_NV12, avdev::PixelFormat::NV12 },
			{ MFVideoFormat_UYVY, avdev::PixelFormat::UYVY },
			{ MFVideoFormat_Y41P, avdev::PixelFormat::Y41P },
			{ MFVideoFormat_Y41T, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_Y42T, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_YUY2, avdev::PixelFormat::YUY2 },
			{ MFVideoFormat_YV12, avdev::PixelFormat::YV12 },

			/* YUV Formats: 10 Bit and 16 Bit */
			{ MFVideoFormat_P010, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_P016, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_P210, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_P216, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_v210, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_v216, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_v410, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_Y210, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_Y216, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_Y410, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_Y416, avdev::PixelFormat::UNKNOWN },

			/* Encoded Video Types */
			{ MFVideoFormat_DV25,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_DV50,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_DVC,   avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_DVH1,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_DVHD,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_DVSD,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_DVSL,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_H264,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_M4S2,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_MJPG,  avdev::PixelFormat::MJPG },
			{ MFVideoFormat_MP43,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_MP4S,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_MP4V,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_MPEG2, avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_MPG1,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_MSS1,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_MSS2,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_WMV1,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_WMV2,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_WMV3,  avdev::PixelFormat::UNKNOWN },
			{ MFVideoFormat_WVC1,  avdev::PixelFormat::UNKNOWN }
		};


		const PictureControlMap getPictureControlMap()
		{
			return pictureControlMap;
		}

		const CameraControlMap getCameraControlMap()
		{
			return cameraControlMap;
		}

		const SampleFormat toSampleFormat(const GUID & apiFormat, UINT32 bitsPerSample)
		{
			if (apiFormat == MFAudioFormat_Float) {
				if (bitsPerSample == 32) {
					return SampleFormat::FLOAT32LE;
				}
			}
			else if (apiFormat == MFAudioFormat_PCM) {
				switch (bitsPerSample) {
					case 8:
						return SampleFormat::U8;
					case 16:
						return SampleFormat::S16LE;
					case 24:
						return SampleFormat::S24LE;
					case 32:
						return SampleFormat::S32LE;
				}
			}
			
			std::string str = GUIDtoString(apiFormat);

			throw AVdevException("No mapping for sample format: %s.", str.c_str());
		}

		const GUID toApiType(const SampleFormat & format)
		{
			switch (format) {
				case SampleFormat::FLOAT32BE:
				case SampleFormat::FLOAT32LE:
					return MFAudioFormat_Float;
				
				case SampleFormat::U8:
				case SampleFormat::S16BE:
				case SampleFormat::S16LE:
				case SampleFormat::S24BE:
				case SampleFormat::S24LE:
				case SampleFormat::S32BE:
				case SampleFormat::S32LE:
					return MFAudioFormat_PCM;

				default:
					throw AVdevException("No mapping for sample format: %d.", format);
			}
		}

		const PixelFormat toPixelFormat(const GUID & apiFormat)
		{
			auto got = pixelFormatMap.find(apiFormat);

			if (got == pixelFormatMap.end()) {
				std::string str = GUIDtoString(apiFormat);

				throw AVdevException("No mapping for pixel format: %s.", str.c_str());
			}

			return got->second;
		}

		const GUID toApiType(const PixelFormat & format)
		{
			auto got = std::find_if(pixelFormatMap.begin(), pixelFormatMap.end(),
				[format](const std::pair<const GUID, const PixelFormat> & vt) { return vt.second == format; });

			if (got == pixelFormatMap.end()) {
				throw AVdevException("No mapping for pixel format: %s.", PixelFormatToString(format).c_str());
			}

			return got->first;
		}

		const PictureControlType toPictureControlType(const KSPROPERTY_VIDCAP_VIDEOPROCAMP & apiType)
		{
			auto got = pictureControlMap.find(apiType);

			if (got == pictureControlMap.end()) {
				throw AVdevException("No mapping for picture control type: %d.", apiType);
			}

			return got->second;
		}

		const KSPROPERTY_VIDCAP_VIDEOPROCAMP toApiType(const PictureControlType & type)
		{
			auto got = std::find_if(pictureControlMap.begin(), pictureControlMap.end(),
				[type](const std::pair<const KSPROPERTY_VIDCAP_VIDEOPROCAMP, const PictureControlType> & vt) { return vt.second == type; });

			if (got == pictureControlMap.end()) {
				throw AVdevException("No mapping for picture control type: %d.", type);
			}

			return got->first;
		}

		const CameraControlType toCameraControlType(const KSPROPERTY_VIDCAP_CAMERACONTROL & apiType)
		{
			auto got = cameraControlMap.find(apiType);

			if (got == cameraControlMap.end()) {
				throw AVdevException("No mapping for camera control type: %d.", apiType);
			}

			return got->second;
		}

		const KSPROPERTY_VIDCAP_CAMERACONTROL toApiType(const CameraControlType & type)
		{
			auto got = std::find_if(cameraControlMap.begin(), cameraControlMap.end(),
				[type](const std::pair<const KSPROPERTY_VIDCAP_CAMERACONTROL, const CameraControlType> & vt) { return vt.second == type; });

			if (got == cameraControlMap.end()) {
				throw AVdevException("No mapping for camera control type: %d.", type);
			}

			return got->first;
		}
	}
}