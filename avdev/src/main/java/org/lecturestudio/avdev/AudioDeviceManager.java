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

public class AudioDeviceManager extends DeviceManager {

	native public AudioCaptureDevice getDefaultAudioCaptureDevice();
	native public AudioPlaybackDevice getDefaultAudioPlaybackDevice();

	native public List<AudioCaptureDevice> getAudioCaptureDevices();
	native public List<AudioPlaybackDevice> getAudioPlaybackDevices();



	private static final class InstanceHolder {

		static final AudioDeviceManager INSTANCE = init();

		private static AudioDeviceManager init() {
			try {
				return new AudioDeviceManager();
			}
			catch (Exception e) {
				throw new RuntimeException("Create audio device manager failed", e);
			}
		}
	}


	public static AudioDeviceManager getInstance() {
		return InstanceHolder.INSTANCE;
	}

	private AudioDeviceManager() {

	}
}
