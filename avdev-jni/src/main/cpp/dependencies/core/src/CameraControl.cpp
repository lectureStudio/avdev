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

#include "CameraControl.h"

namespace avdev
{
	CameraControl::CameraControl(CameraControlType type, long min, long max, long step, long def, bool autoMode) :
		VideoControl(type, min, max, step, def, autoMode)
	{
	}
	
	std::string CameraControl::getName() const
	{
		switch (type) {
			case CameraControlType::Pan:
				return "Pan";
			case CameraControlType::Tilt:
				return "Tilt";
			case CameraControlType::Roll:
				return "Roll";
			case CameraControlType::Zoom:
				return "Zoom";
			case CameraControlType::Exposure:
				return "Exposure";
			case CameraControlType::Iris:
				return "Iris";
			case CameraControlType::Focus:
				return "Focus";
			case CameraControlType::ScanMode:
				return "Scan Mode";
			case CameraControlType::Privacy:
				return "Privacy";
			case CameraControlType::PanTilt:
				return "Pan Tilt";
			case CameraControlType::PanRelative:
				return "Pan Relative";
			case CameraControlType::TiltRelative:
				return "Tilt Relative";
			case CameraControlType::RollRelative:
				return "Roll Relative";
			case CameraControlType::ZoomRelative:
				return "Zoom Relative";
			case CameraControlType::ExposureRelative:
				return "Exposure Relative";
			case CameraControlType::IrisRelative:
				return "Iris Relative";
			case CameraControlType::FocusRelative:
				return "Focus Relative";
			case CameraControlType::PanTiltRelative:
				return "Pan Tilt Relative";
			case CameraControlType::FocalLength:
				return "Focal Length";
			case CameraControlType::AutoExposurePriority:
				return "Auto Exposure Priority";
			case CameraControlType::JpegChromaSubsampling:
				return "Jpeg Chroma Subsampling";
			case CameraControlType::JpegRestartInterval:
				return "Jpeg Restart Interval";
			case CameraControlType::JpegCompressionQuality:
				return "Jpeg Compression Quality";
			case CameraControlType::JpegActiveMarker:
				return "Jpeg Active Marker";
			case CameraControlType::LedMode:
				return "LED Mode";
			case CameraControlType::LedFrequency:
				return "LED Frequency";
			default:
				return "Unknown Control";
		}
	}
}