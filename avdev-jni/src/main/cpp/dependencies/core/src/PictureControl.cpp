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

#include "PictureControl.h"

namespace avdev
{
	PictureControl::PictureControl(PictureControlType type, long min, long max, long step, long def, bool autoMode) :
		VideoControl(type, min, max, step, def, autoMode)
	{
	}

	std::string PictureControl::getName() const
	{
		switch (type) {
			case PictureControlType::Brightness:
				return "Brightness";
			case PictureControlType::Contrast:
				return "Contrast";
			case PictureControlType::Hue:
				return "Hue";
			case PictureControlType::Exposure:
				return "Exposure";
			case PictureControlType::Saturation:
				return "Saturation";
			case PictureControlType::Sharpness:
				return "Sharpness";
			case PictureControlType::Gamma:
				return "Gamma";
			case PictureControlType::ColorEnable:
				return "Color Enable";
			case PictureControlType::BacklightCompensation:
				return "Backlight Compensation";
			case PictureControlType::Gain:
				return "Gain";
			case PictureControlType::DigitalMultiplier:
				return "Digital Multiplier";
			case PictureControlType::DigitalMultiplierLimit:
				return "Digital Multiplier Limit";
			case PictureControlType::WhiteBalance:
				return "White Balance";
			case PictureControlType::WhiteBalanceComponent:
				return "White Balance Component";
			case PictureControlType::PowerLineFrequency:
				return "Power Line Frequency";
			default:
				return "Unknown Control";
		}
	}
}