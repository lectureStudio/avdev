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

package org.lecturestudio.avdev;

import java.util.HashMap;
import java.util.Map;

public class PictureControl extends VideoControl<PictureControlType> {

	private static final Map<PictureControlType, String> nameMap;
	
	static
    {
		nameMap = new HashMap<>();
		nameMap.put(PictureControlType.BacklightCompensation,  "backlight.compensation");
		nameMap.put(PictureControlType.Brightness,             "brightness");
		nameMap.put(PictureControlType.ColorEnable,            "color.enable");
		nameMap.put(PictureControlType.Contrast,               "contrast");
		nameMap.put(PictureControlType.DigitalMultiplier,      "digital.multiplier");
		nameMap.put(PictureControlType.DigitalMultiplierLimit, "digital.multiplier.limit");
		nameMap.put(PictureControlType.Exposure,               "exposure");
		nameMap.put(PictureControlType.Gain,                   "gain");
		nameMap.put(PictureControlType.Gamma,                  "gamma");
		nameMap.put(PictureControlType.Hue,                    "hue");
		nameMap.put(PictureControlType.PowerLineFrequency,     "powerline.frequency");
		nameMap.put(PictureControlType.Saturation,             "saturation");
		nameMap.put(PictureControlType.Sharpness,              "sharpness");
		nameMap.put(PictureControlType.WhiteBalance,           "whitebalance");
		nameMap.put(PictureControlType.WhiteBalanceComponent,  "whitebalance.component");
		nameMap.put(PictureControlType.NotImplemented,         "not.implemented");
    }
	
	public PictureControl(PictureControlType type, long min, long max, long step, long def, boolean autoMode) {
		super(type, min, max, step, def, autoMode);
	}

	@Override
	public String getName() {
		return nameMap.get(getType());
	}
	
}
