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

#ifndef AVDEV_CORE_CAMERA_CONTROL_H_
#define AVDEV_CORE_CAMERA_CONTROL_H_

#include "VideoControl.h"

namespace avdev
{
	enum class CameraControlType
	{
		Pan,
		Tilt,
		Roll,
		Zoom,
		AutoExposurePriority,
		Exposure,
		Iris,
		Focus,
		ScanMode,
		Privacy,
		PanTilt,
		PanRelative,
		TiltRelative,
		RollRelative,
		ZoomRelative,
		ExposureRelative,
		IrisRelative,
		FocusRelative,
		PanTiltRelative,
		FocalLength,
		JpegChromaSubsampling,
		JpegRestartInterval,
		JpegCompressionQuality,
		JpegActiveMarker,
		LedMode,
		LedFrequency,
		NotImplemented
	};
	
	class CameraControl : public VideoControl<CameraControlType>
	{
		public:
			CameraControl(CameraControlType type, long min, long max, long step, long def, bool autoMode);
			std::string getName() const;
	};
}

#endif