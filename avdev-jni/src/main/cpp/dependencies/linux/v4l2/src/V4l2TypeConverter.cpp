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

#include "AVdevException.h"
#include "V4l2TypeConverter.h"

namespace avdev
{
	namespace V4l2TypeConverter
	{
		const std::uint64_t V4L2_CID_LOGITECH_BASE           = 0x0A046D01;
		const std::uint64_t V4L2_CID_LOGITECH_LED1_MODE      = V4L2_CID_LOGITECH_BASE + 4;
		const std::uint64_t V4L2_CID_LOGITECH_LED1_FREQUENCY = V4L2_CID_LOGITECH_BASE + 5;
		
		PictureControlMap pictureControlMap =
		{
			{ V4L2_CID_BACKLIGHT_COMPENSATION,    PictureControlType::BacklightCompensation },
			{ V4L2_CID_BRIGHTNESS,                PictureControlType::Brightness },
			{ V4L2_CID_CONTRAST,                  PictureControlType::Contrast },
			{ V4L2_CID_SATURATION,                PictureControlType::Saturation },
			{ V4L2_CID_HUE,                       PictureControlType::Hue },
			{ V4L2_CID_EXPOSURE,                  PictureControlType::Exposure },
			{ V4L2_CID_AUTO_WHITE_BALANCE,        PictureControlType::WhiteBalance },
			{ V4L2_CID_WHITE_BALANCE_TEMPERATURE, PictureControlType::WhiteBalanceComponent },
			{ V4L2_CID_RED_BALANCE,               PictureControlType::NotImplemented },
			{ V4L2_CID_BLUE_BALANCE,              PictureControlType::NotImplemented },
			{ V4L2_CID_GAMMA,                     PictureControlType::Gamma },
			{ V4L2_CID_GAIN,                      PictureControlType::Gain },
			{ V4L2_CID_HFLIP,                     PictureControlType::NotImplemented },
			{ V4L2_CID_VFLIP,                     PictureControlType::NotImplemented },
			{ V4L2_CID_POWER_LINE_FREQUENCY,      PictureControlType::PowerLineFrequency },
			{ V4L2_CID_SHARPNESS,                 PictureControlType::Sharpness },
			{ V4L2_CID_CHROMA_AGC,                PictureControlType::NotImplemented },
			{ V4L2_CID_COLOR_KILLER,              PictureControlType::NotImplemented },
			{ V4L2_CID_COLORFX,                   PictureControlType::NotImplemented },
			{ V4L2_CID_AUTOBRIGHTNESS,            PictureControlType::NotImplemented },
			{ V4L2_CID_BAND_STOP_FILTER,          PictureControlType::NotImplemented },
			{ V4L2_CID_ROTATE,                    PictureControlType::NotImplemented },
			{ V4L2_CID_BG_COLOR,                  PictureControlType::NotImplemented },
			{ V4L2_CID_CHROMA_GAIN,               PictureControlType::NotImplemented },
			{ V4L2_CID_ILLUMINATORS_1,            PictureControlType::NotImplemented },
			{ V4L2_CID_ILLUMINATORS_2,            PictureControlType::NotImplemented },
			{ V4L2_CID_MIN_BUFFERS_FOR_CAPTURE,   PictureControlType::NotImplemented },
			{ V4L2_CID_MIN_BUFFERS_FOR_OUTPUT,    PictureControlType::NotImplemented },
			{ V4L2_CID_ALPHA_COMPONENT,           PictureControlType::NotImplemented },
			{ V4L2_CID_COLORFX_CBCR,              PictureControlType::NotImplemented }
		};
		
		CameraControlMap cameraControlMap =
		{
			{ V4L2_CID_EXPOSURE_ABSOLUTE,           CameraControlType::Exposure },
			{ V4L2_CID_EXPOSURE_AUTO_PRIORITY,      CameraControlType::AutoExposurePriority },
			{ V4L2_CID_EXPOSURE_METERING,           CameraControlType::NotImplemented },
			{ V4L2_CID_PAN_RELATIVE,                CameraControlType::PanRelative },
			{ V4L2_CID_TILT_RELATIVE,               CameraControlType::TiltRelative },
			{ V4L2_CID_PAN_ABSOLUTE,                CameraControlType::Pan },
			{ V4L2_CID_TILT_ABSOLUTE,               CameraControlType::Tilt },
			{ V4L2_CID_FOCUS_ABSOLUTE,              CameraControlType::Focus },
			{ V4L2_CID_FOCUS_RELATIVE,              CameraControlType::FocusRelative },
			{ V4L2_CID_ZOOM_ABSOLUTE,               CameraControlType::Zoom },
			{ V4L2_CID_ZOOM_RELATIVE,               CameraControlType::ZoomRelative },
			{ V4L2_CID_ZOOM_CONTINUOUS,             CameraControlType::NotImplemented },
			{ V4L2_CID_IRIS_ABSOLUTE,               CameraControlType::Iris },
			{ V4L2_CID_IRIS_RELATIVE,               CameraControlType::IrisRelative },
			{ V4L2_CID_PRIVACY,                     CameraControlType::Privacy },
			{ V4L2_CID_AUTO_EXPOSURE_BIAS,          CameraControlType::NotImplemented },
			{ V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, CameraControlType::NotImplemented },
			{ V4L2_CID_WIDE_DYNAMIC_RANGE,          CameraControlType::NotImplemented },
			{ V4L2_CID_IMAGE_STABILIZATION,         CameraControlType::NotImplemented },
			{ V4L2_CID_ISO_SENSITIVITY,             CameraControlType::NotImplemented },
			{ V4L2_CID_ISO_SENSITIVITY_AUTO,        CameraControlType::NotImplemented },
			{ V4L2_CID_SCENE_MODE,                  CameraControlType::NotImplemented },
			{ V4L2_CID_3A_LOCK,                     CameraControlType::NotImplemented },
			{ V4L2_CID_AUTO_FOCUS_START,            CameraControlType::NotImplemented },
			{ V4L2_CID_AUTO_FOCUS_STOP,             CameraControlType::NotImplemented },
			{ V4L2_CID_AUTO_FOCUS_STATUS,           CameraControlType::NotImplemented },
			{ V4L2_CID_AUTO_FOCUS_RANGE,            CameraControlType::NotImplemented },
			{ V4L2_CID_JPEG_CHROMA_SUBSAMPLING,     CameraControlType::JpegChromaSubsampling },
			{ V4L2_CID_JPEG_RESTART_INTERVAL,       CameraControlType::JpegRestartInterval },
			{ V4L2_CID_JPEG_COMPRESSION_QUALITY,    CameraControlType::JpegCompressionQuality },
			{ V4L2_CID_JPEG_ACTIVE_MARKER,          CameraControlType::JpegActiveMarker },
			{ V4L2_CID_LOGITECH_LED1_MODE,          CameraControlType::LedMode },
			{ V4L2_CID_LOGITECH_LED1_FREQUENCY,     CameraControlType::LedFrequency }
		};
			
		PixelFormatMap pixelFormatMap =
		{
			{ V4L2_PIX_FMT_RGB332,  PixelFormat::RGB332 },
			{ V4L2_PIX_FMT_RGB555, 	PixelFormat::RGB555 },
			{ V4L2_PIX_FMT_RGB565,  PixelFormat::RGB565 },
			{ V4L2_PIX_FMT_BGR24,   PixelFormat::BGR24 },
			{ V4L2_PIX_FMT_RGB24,   PixelFormat::RGB24 },
			{ V4L2_PIX_FMT_BGR32,   PixelFormat::BGR32 },
			{ V4L2_PIX_FMT_RGB32,   PixelFormat::RGB32 },
			{ V4L2_PIX_FMT_GREY,    PixelFormat::GREY },
			{ V4L2_PIX_FMT_YVU410,  PixelFormat::YVU410 },
			{ V4L2_PIX_FMT_YVU420,  PixelFormat::YVU420 },
			{ V4L2_PIX_FMT_YUYV,    PixelFormat::YUYV },
			{ V4L2_PIX_FMT_UYVY,    PixelFormat::UYVY },
			{ V4L2_PIX_FMT_YUV422P, PixelFormat::YUV422P },
			{ V4L2_PIX_FMT_YUV411P, PixelFormat::YUV411P },
			{ V4L2_PIX_FMT_Y41P,    PixelFormat::Y41P },
			{ V4L2_PIX_FMT_NV12,    PixelFormat::NV12 },
			{ V4L2_PIX_FMT_NV21,    PixelFormat::NV21 },
			{ V4L2_PIX_FMT_YUV410,  PixelFormat::YUV410 },
			{ V4L2_PIX_FMT_YUV420,  PixelFormat::I420 },
			{ V4L2_PIX_FMT_HI240,   PixelFormat::HI240 },
			{ V4L2_PIX_FMT_MJPEG,   PixelFormat::MJPG },
			{ V4L2_PIX_FMT_JPEG,    PixelFormat::JPEG },
			{ V4L2_PIX_FMT_DV,      PixelFormat::DV },
			{ V4L2_PIX_FMT_MPEG,    PixelFormat::MPEG },
			{ V4L2_PIX_FMT_WNVA,    PixelFormat::WNVA }	
		};
		
			
		const PixelFormat toPixelFormat(const std::uint32_t & apiformat)
		{
			auto got = pixelFormatMap.find(apiformat);

			if (got == pixelFormatMap.end()) {
				throw AVdevException("No mapping for pixel format: %s.", ToFccString(apiformat).c_str());
			}
			
			return got->second;
		}
			
		const std::uint32_t toApiType(const PixelFormat & format)
		{
			auto got = std::find_if(pixelFormatMap.begin(), pixelFormatMap.end(),
				[format](const std::pair<const std::uint32_t, const PixelFormat> & vt) { return vt.second == format; });
			
			if (got == pixelFormatMap.end()) {
					throw AVdevException("No mapping for pixel format: %s.", PixelFormatToString(format).c_str());
			}
			
			return got->first;
		}
			
		const PictureControlType toPictureControlType(const std::uint32_t & apiType)
		{
			auto got = pictureControlMap.find(apiType);

			if (got == pictureControlMap.end()) {
				throw AVdevException("No mapping for picture control type: %d.", apiType);
			}
			
			return got->second;
		}
			
		const std::uint32_t toApiType(const PictureControlType & type)
		{
			auto got = std::find_if(pictureControlMap.begin(), pictureControlMap.end(),
				[type](const std::pair<const std::uint32_t, const PictureControlType> & vt) { return vt.second == type; });
			
			if (got == pictureControlMap.end()) {
					throw AVdevException("No mapping for picture control type.");
			}
			
			return got->first;
		}
		
		const CameraControlType toCameraControlType(const std::uint32_t & apiType)
		{
			auto got = cameraControlMap.find(apiType);

			if (got == cameraControlMap.end()) {
				throw AVdevException("No mapping for camera control type: %d.", apiType);
			}
			
			return got->second;
		}
			
		const std::uint32_t toApiType(const CameraControlType & type)
		{
			auto got = std::find_if(cameraControlMap.begin(), cameraControlMap.end(),
				[type](const std::pair<const std::uint32_t, const CameraControlType> & vt) { return vt.second == type; });
			
			if (got == cameraControlMap.end()) {
					throw AVdevException("No mapping for camera control type.");
			}
			
			return got->first;
		}

	}
}
