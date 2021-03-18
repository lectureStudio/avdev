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

public class CameraControl extends VideoControl<CameraControlType> {
	
	private static final Map<CameraControlType, String> nameMap;
	
	static
    {
		nameMap = new HashMap<>();
		nameMap.put(CameraControlType.AutoExposurePriority,   "auto.exposure.priority");
		nameMap.put(CameraControlType.Exposure,               "exposure");
		nameMap.put(CameraControlType.ExposureRelative,       "exposure.relative");
		nameMap.put(CameraControlType.FocalLength,            "focal.length");
		nameMap.put(CameraControlType.Focus,                  "focus");
		nameMap.put(CameraControlType.FocusRelative,          "focus.relative");
		nameMap.put(CameraControlType.Iris,                   "iris");
		nameMap.put(CameraControlType.IrisRelative,           "iris.relative");
		nameMap.put(CameraControlType.JpegActiveMarker,       "jpeg.active.marker");
		nameMap.put(CameraControlType.JpegChromaSubsampling,  "jpeg.chroma.subsampling");
		nameMap.put(CameraControlType.JpegCompressionQuality, "jpeg.compression.quality");
		nameMap.put(CameraControlType.JpegRestartInterval,    "jpeg.restart.interval");
		nameMap.put(CameraControlType.LedFrequency,           "led.frequency");
		nameMap.put(CameraControlType.LedMode,                "led.mode");
		nameMap.put(CameraControlType.Pan,                    "pan");
		nameMap.put(CameraControlType.PanRelative,            "pan.relative");
		nameMap.put(CameraControlType.PanTilt,                "pan.tilt");
		nameMap.put(CameraControlType.PanTiltRelative,        "pan.tilt.relative");
		nameMap.put(CameraControlType.Privacy,                "privacy");
		nameMap.put(CameraControlType.Roll,                   "roll");
		nameMap.put(CameraControlType.RollRelative,           "roll.relative");
		nameMap.put(CameraControlType.ScanMode,               "scan.mode");
		nameMap.put(CameraControlType.Tilt,                   "tilt");
		nameMap.put(CameraControlType.TiltRelative,           "tilt.relative");
		nameMap.put(CameraControlType.Zoom,                   "zoom");
		nameMap.put(CameraControlType.ZoomRelative,           "zoom.relative");
    }
	
	public CameraControl(CameraControlType type, long min, long max, long step, long def, boolean autoMode) {
		super(type, min, max, step, def, autoMode);
	}

	@Override
	public String getName() {
		return nameMap.get(getType());
	}

}
