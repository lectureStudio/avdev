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

import java.util.List;

public class VideoCaptureDevice extends Device {

	native public VideoOutputStream createOutputStream(VideoSink sink);
	
	native public List<PictureFormat> getPictureFormats();
	native public List<PictureControl> getPictureControls();
	native public List<CameraControl> getCameraControls();

	native public void setPictureControlAutoMode(PictureControlType type, boolean auto);
	native public boolean getPictureControlAutoMode(PictureControlType type);
	native public void setPictureControlValue(PictureControlType type, long value);
	native public long getPictureControlValue(PictureControlType type);

	native public void setCameraControlAutoMode(CameraControlType type, boolean auto);
	native public boolean getCameraControlAutoMode(CameraControlType type);
	native public void setCameraControlValue(CameraControlType type, long value);
	native public long getCameraControlValue(CameraControlType type);
	
	native public void setFrameRate(float frameRate);
	native public float getFrameRate();

	native public void setPictureFormat(PictureFormat format);
	native public PictureFormat getPictureFormat();
	
	
	private VideoCaptureDevice() {

	}
	
}
